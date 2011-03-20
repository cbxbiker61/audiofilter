#pragma once
#ifndef AUDIOFILTER_PARSER_H
#define AUDIOFILTER_PARSER_H

/*
 * Abstract parser interface
 */

#include <string>
#include <vector>
#include "BitStream.h"
#include "Buffer.h"
#include "Speakers.h"
#include "Ac3Defs.h"
#include "IMDCT.h"

namespace AudioFilter {

struct HeaderInfo;
class HeaderParser;
class FrameParser;

class StreamBuffer;

///////////////////////////////////////////////////////////////////////////////
// HeaderParser
// HeaderInfo
//
// Abstract interface for scanning and detecting compressed stream.
// Header information structure.
//
// Sometimes we need to scan the stream without actual decoding. Of course,
// parser can do this, but we don't need most of its features, buffers, tables,
// etc in this case. For example, application that just detects format of a
// compressed stream will contain all the code required to decode it and create
// unneeded memory buffers. To avoid this we need a lightweight interface to
// work only with frame headers.
//
// Therefore implementation of HeaderParser should be separated from other
// parser parts so we can use it alone. Ideally it should be 2 files (.h and
// .cpp) without other files required.
//
// Header parser is a class without internal state, just a set of functoins.
// So we may create one constant class and use it everywhere. Therefore all
// pointers to HeaderParser objects are const.
//
// Frame parser should return a pointer to a header parser that corresponds
// to the given frame parser.
//
// HeaderParser interface allows nesting, so several header parsers may be
// represented as one parser (see MultiHeaderParser class). Therefore instead of
// list of parsers we can use only one parser pointer everywhere.
//
///////////////////////////////////////////////////////////////////////////////
// HeaderInfo
//
// Syncronization
// ==============
//
// We distinguish 2 stream types:
// * Stream with known frame size. For this strean kind we can determine the
//   frame size by parsing the header. It may be variable bitrate stream with
//   different frame sizes.
// * Stream with unknown frame size (free-format). For this type of stream
//   frame size is unknown from the header. So we must determine inter-frame
//   distance. After that we suppose that frame size is constant and expect
//   next header right after the end of the frame. Threfore, free-format stream
//   is always constant-bitrate.
//
// Sparse stream is the stream where some gap (or padding) is present in
// between of two frames. To locate next syncpoint we should scan input data
// after the end of the first frame to locate the next. To limit such scanning
// we should know how much data to scan. Parser can specify the exact amount
// of data to scan after the current frame.
//
// Only known frame size stream is allowed to be sparse (because free-format
// stream is known to have the same frame size). But free-format stream can
// specify scan_size to limit scanning during inter-frame interval detection.
// If scan_size for free-format stream is unspecified we should scan up to
// max_frame_size.
//
// spk
//   Format of the stream. FORMAT_UNKNOWN indicates a parsing error.
//
// frame_size
//   Frame size (including the header):
//   0  - frame size is unknown (free-format stream)
//   >0 - known frame size
//
// scan_size
//   Use scanning to locate next syncpoint
//   0  - do not use scanning
//   >0 - maximum distance between syncpoints
//
// nsamples
//   Number of samples at the given frame.
//
//   We can derive current bitrate for known frame size stream as follows:
//   (*) bitrate = spk.sample_rate * frame_size * 8 / nsamples;
//   But note actual bitrate may be larger for sparce stream.
//
// bs_type
//   Bitstream type. BITSTREAM_XXXX constants.
//
// spdif_type
//   If given format is spdifable it defines spdif packet type (Pc burst-info).
//   Zero otherwise. This field may be used to determine spdifable format.
//
///////////////////////////////////////////////////////////////////////////////
// HeaderParser
//
// header_size()
//   Minimum number of bytes required to parse header.
//
// min_frame_size()
//   Minimum frame size possible. Must be >= header size.
//
// max_frame_size()
//   Maximum frame size possible. Must be >= minimum frame size.
//   Note that for spdifable formats we must take in account maximum spdif
//   frame size to be able to parse spdif-padded format.
//
// can_parse()
//   Determine that we can parse the format given. (Or, that parser can detect
//   this format). For example if some_parser.can_parse(FORMAT_AC3) == true
//   this parser can parse ac3 format, or can detect in in raw data.
//
// parse_header()
//   Parse header and write header information.
//
//   Size of header buffer given must be >= header_size() (it is not verified
//   and may lead to memory fault).
//
// compare_headers()
//   Check that both headers belong to the same stream. Some compressed formats
//   may determine stream changes with some additional header info (not only
//   frame size and stream format). Headers given must be correct headers
//   checked with parse() call, so this call may not do all headers checks.
//   Note that when headers are equal, format of both headers must be same:
//
//   parse_header(phdr1, hdr1);
//   parse_header(phdr2, hdr2);
//   if (compare_headers(phdr1, phdr2))
//   {
//     assert(hdr1.spk == hdr2.spk);
//     assert(hdr1.bs_type == hdr2.bs_type);
//     assert(hdr1.spdif_type == hdr2.spdif_type);
//     // frame size may differ for variable bitrate
//     // nsamples may differ
//   }
//
//   Size of header buffers given must be >= header_size() (it is not verified
//   and may lead to memory fault).
//
// header_info()
//   Dump stream information that may be useful to track problems. Default
//   implementation shows HeaderInfo parameters.
//
//   Size of header buffer given must be >= header_size() (it is not verified
//   and may lead to memory fault).

struct HeaderInfo
{
  HeaderInfo()
    : spk(spk_unknown)
    , frame_size(0)
    , hd_size(0)
    , scan_size(0)
    , nsamples(0)
    , bs_type(0)
    , spdif_type(0)
  {}

  void drop(void)
  {
    spk = spk_unknown;
    frame_size = 0;
    scan_size = 0;
    nsamples = 0;
    bs_type = BITSTREAM_NONE;
    spdif_type = 0;
  }

  bool isHd(void)
  {
      return hd_size != 0;
  }

  Speakers spk;
  size_t frame_size;
  size_t hd_size;
  size_t scan_size;
  size_t nsamples;
  int bs_type;
  uint16_t spdif_type;
};

class HeaderParser
{
public:
  HeaderParser() {}
  virtual ~HeaderParser() {}

  virtual size_t getHeaderSize(void) const = 0;
  virtual size_t getMinFrameSize(void) const = 0;
  virtual size_t getMaxFrameSize(void) const = 0;
  virtual bool canParse(int format) = 0;
  virtual bool parseHeader(const uint8_t *hdr, HeaderInfo *hi = 0) = 0;
  virtual bool compareHeaders(const uint8_t *hdr1, const uint8_t *hdr2) = 0;
  virtual std::string getHeaderInfo(const uint8_t *hdr);
};

class Ac3HeaderParser : public HeaderParser
{
public:
  Ac3HeaderParser() {}

  virtual size_t getHeaderSize(void) const
  {
    return 8;
  }

  virtual size_t getMinFrameSize(void) const
  {
    return 128;
  }

  virtual size_t getMaxFrameSize(void) const
  {
    return 3814;
  }

  virtual bool canParse(int format)
  {
    return format == FORMAT_AC3;
  }

  virtual bool parseHeader(const uint8_t *hdr, HeaderInfo *hi = 0);
  virtual bool compareHeaders(const uint8_t *hdr1, const uint8_t *hdr2);
};

class DtsHeaderParser : public HeaderParser
{
public:
  DtsHeaderParser() {}

  virtual size_t getHeaderSize(void) const
  {
    //return 16;
    // This may need to be increased for other Dts formats
    // right now the assumption is that we may have DtsHd
    // streams that place the hd header after the core Dts data
    // Assuming we have a typical dts core of 512 samples
    // this should be enough
    return 2048;
  }

  virtual size_t getMinFrameSize(void) const
  {
    return 96;
  }

  virtual size_t getMaxFrameSize(void) const
  {
    return 16384;
  }

  virtual bool canParse(int format)
  {
    return format == FORMAT_DTS;
  }

  virtual bool parseHeader(const uint8_t *hdr, HeaderInfo *hi = 0);
  virtual bool compareHeaders(const uint8_t *hdr1, const uint8_t *hdr2);
};

class DtsHdHeaderParser : public HeaderParser
{
public:
  DtsHdHeaderParser() {}

  virtual size_t getHeaderSize(void) const
  {
    return 16; // don't know if this is appropriate yet
  }

  virtual size_t getMinFrameSize(void) const
  {
    return 96; // don't know if this is appropriate yet
  }

  virtual size_t getMaxFrameSize(void) const
  {
    return 16384; // don't know if this is appropriate yet
  }

  virtual bool canParse(int format)
  {
    return format == FORMAT_DTS;
  }

  virtual bool parseHeader(const uint8_t *hdr, HeaderInfo *hi = 0);
  virtual bool compareHeaders(const uint8_t *hdr1, const uint8_t *hdr2);
};

class MpaHeaderParser : public HeaderParser
{
public:
  MpaHeaderParser() {}

  virtual size_t getHeaderSize(void) const
  {
    return 4;
  }

  virtual size_t getMinFrameSize(void) const
  {
    return 32;
  }

  virtual size_t getMaxFrameSize(void) const
  {
    return 1728;
  }

  virtual bool canParse(int format)
  {
    return format == FORMAT_MPA;
  }

  virtual bool parseHeader(const uint8_t *hdr, HeaderInfo *hi = 0);
  virtual bool compareHeaders(const uint8_t *hdr1, const uint8_t *hdr2);
};

class MultiHeaderParser : public HeaderParser
{

public:
  MultiHeaderParser();
  virtual ~MultiHeaderParser();
  //typedef std::vector<const HeaderParser*> HeaderParserVec;
  //MultiHeaderParser(const HeaderParserVec&, bool takeParserOwnership = true);
  //void setParsers(const HeaderParserVec&, bool takeParserOwnership = true);
  void addParser(HeaderParser *);
  void releaseParsers(void);

  /////////////////////////////////////////////////////////
  // HeaderParser overrides

  virtual size_t getHeaderSize(void) const
  {
    return f_header_size;
  }

  virtual size_t getMinFrameSize(void) const
  {
    return f_min_frame_size;
  }

  virtual size_t getMaxFrameSize(void) const
  {
    return f_max_frame_size;
  }

  virtual bool canParse(int format);
  virtual bool parseHeader(const uint8_t *hdr, HeaderInfo *hi = 0);
  virtual bool compareHeaders(const uint8_t *hdr1, const uint8_t *hdr2);
  virtual std::string getHeaderInfo(const uint8_t *hdr);

private:
  void zeroSizes(void);
  void initSizes(void);

  typedef std::vector<HeaderParser*> HeaderParserVec;
  HeaderParserVec parserVec;
  HeaderParser *currentParser;
  bool iOwnParsers;
  size_t f_header_size;
  size_t f_min_frame_size;
  size_t f_max_frame_size;
};

///////////////////////////////////////////////////////////////////////////////
// FrameParser
//
// Frame transformation interface. Frame transformations include:
// * Decoders
// * Loseless frame transformations:
//   * Bitstream format change
//   * Gain change
// * Other frame transformations:
//   * Frame wrappers/unwrappers
//   * Bitrate change (without reencoding, aka transcoding)
//   * etc...
// * Stream validations
// * etc...
//
// Unlike HeaderParser interface, FrameParser has an internal state and must
// receive frames in correct order, because frames in a stream are almost always
// related with each other. To prepare parser to receive a new stream and set
// it into initial state we must call reset().
//
// We should reset the transformer with each new stream detected by header
// parser.
//
// All other things are very strightforward: we load a frame with help of
// HeaderParser and give the frame loaded to FrameParser. FrameParser
// does its job and provies access to the transformed data through its
// interface.
//
// FrameParser may do the job in-place at the buffer received with
// parse_frame() call.
//
// header_parser()
//   Returns header parser this tranformer works with.
//
// reset()
//   Set transformer into the initial state. Drop all internal buffers and
//   state variables.
//
// parse_frame()
//   Transform the next frame of a stream.
//   parse_frame() call should not fail if error detected is handled with some
//   kind of restoration procedure that produces some output.
//
// get_spk()
//   Format of a transformed data. FORMAT_UNKNOWN may indicate an error.
//
// get_samples()
//   For linear output format returns pointers to sample buffers.
//   Undefined for non-linear output format.
//
// get_nsamples()
//   For linear output returns numer of samples stored at sample buffers.
//   For non-linear output should indicate number of samples at the output
//   frame or 0 if number of samples is unknown.
//
// get_rawdata()
//   For non-linear output returns pointers to transformed data buffer.
//   For linear output may return a pointer to the original frame data.
//
// get_rawsize()
//   For non-linear output returns size of transformed data.
//   For linear output may return number of bytes actually parsed. It is useful
//   to compact sparse stream.
//
// stream_info()
//   Dump stream information. It may be more informative than header info but
//   should contain only stream-wide information (that does not change from
//   frame to frame).
//
// frame_info()
//   Dump the frame information. Most detailed information for the certain
//   frame. May not include info dumped with stream_info() call.

class FrameParser
{
public:
  FrameParser() {}
  virtual ~FrameParser() {}

  virtual HeaderParser *getHeaderParser(void) = 0;

  /////////////////////////////////////////////////////////
  // Frame transformation

  virtual void reset(void) = 0;
  virtual bool parseFrame(uint8_t *frame, size_t size) = 0;

  /////////////////////////////////////////////////////////
  // Transformed data access

  virtual Speakers getSpk(void) const = 0;
  virtual samples_t getSamples(void) const = 0;
  virtual size_t getNSamples(void) const = 0;
  virtual uint8_t *getRawData(void) const = 0;
  virtual size_t getRawSize(void) const = 0;

  /////////////////////////////////////////////////////////
  // Stream information

  virtual std::string getStreamInfo(void) const = 0;
  virtual std::string getFrameInfo(void) const = 0;
};

// todo: decode_block() for per-block decode

class AC3Info
{
public:
  int       bitrate;     // bitrate     (derived from frame_size & halfrate)
  int       halfrate;    // halfrate    (derived from bsid)

  int       fscod;       // sample rate code
  int       frmsizecod;  // frame size code

  int       bsid;        // bitstream identification
  int       bsmod;       // bitestream mode

  int       acmod;       // audio coding mode
  int       dsurmod;     // dolby surround mode
  bool      lfeon;       // lfe channel present

  sample_t  clev;        // center mix level
  sample_t  slev;        // surround mix level

  bool      compre;      // compression gain word exists
  sample_t  compr;       // compression gain word
  bool      compr2e;     // compression gain word 2 exists
  sample_t  compr2;      // compression gain word 2

  int       dialnorm;    // dialog normalization
  int       dialnorm2;   // dialog normalization 2

  bool      langcode;    // language code exists
  int       langcod;     // language code
  bool      langcod2e;   // language code 2 exists
  int       langcod2;    // language code 2

  bool      audprodie;   // audio production information exists
  int       mixlevel;    // mixing level (SPL)
  int       roomtyp;     // room type

  bool      audprodi2e;  // audio production information 2 exists
  int       mixlevel2;   // mixing level (SPL) 2
  int       roomtyp2;    // room type 2

  bool      copyrightb;  // copyright bit
  bool      origbs;      // original bitstream

  struct {
    int hours;
    int mins;
    int secs;
    int frames;
    int fracs;
  } timecode;            // timecode
};

class AC3FrameState
{
public:
  // general
  bool blksw[5];         // non-reusable between blocks
  bool dithflag[5];      // non-reusable between blocks

  sample_t dynrng;       // needs reset at new frame
  sample_t dynrng2;      // needs reset at new frame

  // coupling info

  bool cplinu;
  bool chincpl[5];
  bool phsflginu;
  int  cplstrtmant;
  int  cplendmant;
  int  ncplbnd;
  int  cplbnd[18];
  sample_t cplco[5][18];

  // channel info

  int rematflg;
  int endmant[5];

  // exponents

  int8_t cplexps[256];
  int8_t exps[5][256];
  int8_t gainrng[5];     // non-reusable between blocks
  int8_t lfeexps[7];

  // bit allocation info

  int sdecay;
  int fdecay;
  int sgain;
  int dbknee;
  int floor;

  int cplsnroffset;
  int snroffset[5];
  int lfesnroffset;

  int cplfgain;
  int fgain[5];
  int lfefgain;

  int cplfleak;
  int cplsleak;

  // delta bit allocation

  int  cpldeltbae;       // needs reset at new frame
  int  deltbae[5];       // needs reset at new frame

  int8_t cpldeltba[50];
  int8_t deltba[5][50];

  // bit allocation

  int8_t cplbap[256];
  int8_t bap[5][256];
  int8_t lfebap[7];
};

class Ac3Parser : public FrameParser, public AC3Info, public AC3FrameState
{
public:
  bool do_crc;        // do crc check
  bool do_dither;     // do dithering
  bool do_imdct;      // do IMDCT

  int frames;
  int errors;

public:
  Ac3Parser();
  ~Ac3Parser();

  /////////////////////////////////////////////////////////
  // FrameParser overrides

  virtual HeaderParser *getHeaderParser(void);

  virtual void reset(void);
  virtual bool parseFrame(uint8_t *frame, size_t size);

  virtual Speakers getSpk(void) const
  {
    return spk;
  }

  virtual samples_t getSamples(void) const
  {
    return samples;
  }

  virtual size_t getNSamples(void) const
  {
    return AC3_FRAME_SAMPLES;
  }

  virtual uint8_t  *getRawData(void) const
  {
    return 0;
  }

  virtual size_t getRawSize(void) const
  {
    return 0;
  }

  virtual std::string getStreamInfo(void) const;
  virtual std::string getFrameInfo(void) const;

protected:

  /////////////////////////////////////////////////////////
  // AC3 parse

  Ac3HeaderParser ac3_header; // header parser
  Speakers  spk;        // output format
  uint8_t  *frame;      // frame data
  size_t    frame_size; // frame size
  int       bs_type;    // bitstream type

  SampleBuf samples;    // samples buffer
  SampleBuf delay;      // delayed samples buffer
  IMDCT     imdct;      // IMDCT
  ReadBS    bs;         // Bitstream reader

  int block;

  bool startParse(uint8_t *frame, size_t size);
  bool checkCrc(void);
  bool decodeBlock(void);
  bool parseHeader(void);
  bool parseBlock(void);
  bool parseExponents(int8_t *exps, int8_t absexp, int expstr, int nexpgrps);
  bool parseDeltba(int8_t *deltba);
  void parseCoeff(samples_t samples);

};

struct SpdifHeaderSync
{
  void set(uint16_t type, uint16_t len)
  {
    _sync1 = 0xf872;
    _sync2 = 0x4e1f;
    _type  = type;
    _len   = len;
  }

  uint16_t _sync1; // Pa sync word 1
  uint16_t _sync2; // Pb sync word 2
  uint16_t _type; // Pc data type
  uint16_t _len; // Pd length, bits or bytes depending on type
};

struct SpdifHeaderSync0
{
  void set(uint16_t type, uint16_t lenBits)
  {
    _z1 = 0;
    _z2 = 0;
    _z3 = 0;
    _z4 = 0;
    _sync.set(type, lenBits);
  }

  uint16_t _z1;
  uint16_t _z2;
  uint16_t _z3;
  uint16_t _z4;

  SpdifHeaderSync _sync;
};

class SpdifHeaderParser : public HeaderParser
{
public:
  SpdifHeaderParser() {}

  /////////////////////////////////////////////////////////
  // HeaderParser overrides
  // Max frame size for SPDIF 8192
  // Header size, min and max frame sizes are determined
  // by the number of samples the frame contains:
  //
  //       header   min    max
  // ------------------------
  // MPA:       4   384    1152
  // AC3:       8  1536    1536
  // DTS:      16   192    4096 (only 2048 is supported by SPDIF)
  //
  // Header size = SPDIF header size + max header size = 32
  // Minimum SPDIF frame size = 192 * 4 = 768
  // Maximum SPDIF frame size = 2048 * 4 = 8192

  virtual size_t getHeaderSize(void) const
  {
    return 32;
  }

  virtual size_t getMinFrameSize(void) const
  {
    return 768;
  }

  virtual size_t getMaxFrameSize(void) const
  {
    return 8192;
  }

  virtual bool canParse(int format)
  {
    return format == FORMAT_SPDIF;
  }

  virtual bool parseHeader(const uint8_t *hdr, HeaderInfo *hi = 0);
  virtual bool compareHeaders(const uint8_t *hdr1, const uint8_t *hdr2);

  virtual std::string getHeaderInfo(const uint8_t *hdr);
};

///////////////////////////////////////////////////////////////////////////////
// StreamBuffer
//
// Implements stream scanning algorithm. This includes reliable synchronization
// algorithm with 3 consecutive syncpoints and frame load algorithm. May be
// used when we need:
// * reliable stream synchronization
// * frame-based stream walk
//
// It is a difference between frame size and frame interval. Frame interval
// is a distance between consecutive syncpoints. Frame size is size of
// frame data. Frame interval may be larger than frame size because of SPDIF
// padding for example.
//
// We can parse 2 types of streams: streams with frame size known from the
// header and streams with unknown frame size.
//
// For known frame size we load only known frame data and all other data is
// skipped. Frame interval is known only after successful frame load and it is
// a distance between *previous* frame header and current frame's header.
//
// To detect stream changes we search for new stream header after the end of
// the frame loaded. If we cannot find a header of the same stream before
// we reach maximum frame size we do resync.
//
// For unknown frame size frame interval is a constant determined at stream
// syncronization procedure. We always load all frame interval data therefore
// frame size and frame interval are always equal in this case and stream is
// known to be of constant bitrate.
//
// If header of the same stream is not found exactly after the end of loaded
// frame we do resync. Therefore this type of the stream cannot contain
// inter-frame debris (see below).
//
// Debris is all data that do not belong to frames. This includes all data that
// is not a compressed stream at all and all inter-frame data.
// Debris is the mechanism to maintain bit-to-bit correctness, so we may
// construct the original stream back from parsed one. Also it allows us to
// create stream detector, so we can process uncompressed data in one way and
// compessed in another. For instance, it is required for PCM/SPDIF detection
// and helps to handle SPDIF <-> PCM transitions in a correct way.
//
// After load() call StreamBuffer may be in following states:
//
// syn new deb frm
//  -   -   -   -   no sync (not enough data) (1)
//  -   -   +   -   no sync with debris output
//  +   -   -   -   no frame loaded (not enough data) (1)
//  +   -   -   +   frame without debris
//  +   -   +   -   inter-frame debris (2)
//  +   -   +   +   frame with debris
//  +   +   -   +   sync on a new stream
//  +   +   +   +   sync on a new stream with debris
// (1) - all input buffer data is known to be processed after load() call
// (2) - state is possible but not used in curent implementation.
//
// Prohibited states:
//
// syn new deb frm
//  -   -   *   +   frame loaded without sync
//  -   +   *   *   new stream without sync
//  +   +   *   -   new stream detection without a frame loaded
//
// load() call returns false in case when stream buffer was not loaded with
// debris or frame data. I.e. in states marked with (1).
//
// load_frame() skips all debris data and loads only frames. It returns false
// in all states without a frame loaded. So only following states are possible
// after load_frame():
//
// syn new frm
//  -   -   -   no sync (not enough data) (1)
//  +   -   -   frame was not loaded (not enough data) (1)
//  +   -   +   frame loaded
//  +   +   +   sync on a new stream
// (1) - all input buffer is known to be processed after load_frame() call
//
// flush() releases all buffered data as debris. Returns true if it is some
// data to flush.
//
// Internal buffer structure
// =========================
//
// StreamBuffer uses 3-point syncronization. This means that we need buffer
// space for 2 full frames and 1 header more. Also, we need some space for a
// copy of a header to load each frame.
//
// +-- header pointer
// V
// +--------------+-----------------------+-----------------------+---------+
// | Header1 copy | Frame1                | Frame2                | Header3 |
// +--------------+-----------------------+-----------------------+---------+
//                ^
//                +-- frame pointer
//
// And total buffer size equals to:
// buffer_size = max_frame_size * 2 + header_size * 2;
//
// Important note!!!
// =================
//
// For unknown frame size and SPDIF stream we cannot load the last frame of
// the stream correctly! (14bit DTS falls into this condition)
//
// If stream ends after the last frame the last frame is not loaded at all.
// It is because we must load full inter-stream interval but we cannot do this
// because SPDIF header is not transmitted after the last frame:
//
//                +------------- frame interval ---------+
//                V                                      V
// +--------------------------------------+--------------------------------------+
// | SPDIF header | Frame1 data | padding | SPDIF header | Frame2 data | padding |
// +--------------------------------------+--------------------------------------+
//                                                       ^                       ^
//                         Less than frame interval -----+-----------------------+
//
// We can correctly handle switch between different types of SPDIF stream, but
// we cannot correctly handle switch between SPDIF with unknown frame size to
// non-SPDIF stream, and we loose at least one frame of a new stream:
//
//                +------------ frame interval --------+
//                V                                    V
// +------------------------------------+------------------------------------+
// | SPDIF header | DTS frame | padding | SPDIF header | AC3 frame | padding |
// +------------------------------------+------------------------------------+
//                                                     ^
//                                                     +-- correct stream switch
//
//                +------------ frame interval --------+
//                V                                    V
// +------------------------------------+----------------------------------+
// | SPDIF header | DTS frame | padding | AC3 frame | AC3Frame | AC3 frame |
// +------------------------------------+----------------------------------+
//                                      ^                      ^
//                                      +---- lost frames -----+--- stream switch
//
// Therefore in spite of the fact that we CAN work with SPDIF stream it is
// recommended to demux SPDIF stream before working with the contained stream.
// We can demux SPDIF stream correctly because SPDIF header contains real
// frame size of the contained stream.

class StreamBuffer
{
public:
  StreamBuffer();
  StreamBuffer(HeaderParser *);
  virtual ~StreamBuffer();

  /////////////////////////////////////////////////////////
  // Init

  bool setParser(HeaderParser *);

  HeaderParser *getParser(void)
  {
    return _parser;
  }

  void releaseParser(void);

  /////////////////////////////////////////////////////////
  // Processing

  void reset(void);
  bool loadFrame(uint8_t **data, uint8_t *end);
  bool flush(void);

  /////////////////////////////////////////////////////////
  // State flags

  bool isInSync(void) const
  {
    return _isInSync;
  }

  bool isNewStream(void) const
  {
    return _isNewStream;
  }

  bool isFrameLoaded(void) const
  {
    return _frame.size  > 0;
  }

  bool isDebrisExists(void) const
  {
    return _debris.size > 0;
  }

  /////////////////////////////////////////////////////////
  // Data access

  const uint8_t *getBuffer(void) const
  {
    return _sync.ptr;
  }

  size_t getBufferSize(void) const
  {
    return _sync.dataSize;
  }

  uint8_t *getDebris(void) const
  {
    return _debris.ptr;
  }

  size_t getDebrisSize(void) const
  {
    return _debris.size;
  }

  Speakers getSpk(void) const
  {
    return _hinfo.spk;
  }

  uint8_t *getFrame(void) const
  {
    return _frame.ptr;
  }

  size_t getFrameSize(void) const
  {
    return _frame.size;
  }

  size_t getFrameInterval(void) const
  {
    return _frame.interval;
  }

  int getFrameCount(void) const
  {
    return _frame.count;
  }

  std::string getStreamInfo(void) const;

  HeaderInfo getHeaderInfo(void)
  {
    return _hinfo;
  }

private:
  bool loadBuffer(uint8_t **data, uint8_t *end, size_t required_size);
  void dropBuffer(size_t size);
  bool reSync(uint8_t **data, uint8_t *data_end);
  bool load(uint8_t **data, uint8_t *end);

  // Parser info (constant)

  HeaderParser *_parser; // header parser
  size_t _headerSize; // cached header size
  size_t _minFrameSize; // cached min frame size
  size_t _maxFrameSize; // cached max frame size

  // Buffers
  // We need a header of a previous frame to load next one, but frame data of
  // the frame loaded may be changed by in-place frame processing. Therefore
  // we have to keep a copy of the header. So we need 2 buffers: header buffer
  // and sync buffer. Header buffer size is always header_size.

  UInt8Buf _buf;

  uint8_t *_headerBuf; // header buffer pointer
  HeaderInfo _hinfo; // header info (parsed header buffer)

  struct Sync
  {
    Sync() : ptr(0), size(0), dataSize(0) {}
    uint8_t *ptr;
    size_t size;
    size_t dataSize;
  } _sync;

  size_t _preFrameSize; // amount of pre-frame data allowed
                    // (see comment to sync() function)

  // Data (frame data and debris)

  struct Debris
  {
    Debris() : ptr(0), size(0) {}
    uint8_t *ptr;
    size_t size;
  } _debris;

  struct Frame
  {
    Frame() : ptr(0), size(0), interval(0), count(0) {}

    uint8_t *ptr; // pointer to the start of the frame
    size_t size; // size of the frame loaded
    size_t interval; // frame interval
    int count; // number of frames loaded
  } _frame;

  bool _isInSync; // we're in sync with the stream
  bool _isNewStream; // frame loaded belongs to a new stream
};

}; // namespace AudioFilter

#endif

// vim: ts=2 sts=2 et

