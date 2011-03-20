/*
  DVDGraph
  Audio decoder and processor with SPDIF support.

  Suports following input formats: PCM/SPDIF/PES/compressed. Supports SPDIF
  passthrough output and AC3 encoding on the fly. Can query a sink about
  format support and find closest match to the user wish. Can detect SPDIF
  stream in PCM data (WAV/DTS format).  
*/


#ifndef VALIB_DVD_GRAPH_H
#define VALIB_DVD_GRAPH_H

#include "../filter_graph.h"
#include "../parsers/ac3/ac3_enc.h"
#include "demux.h"
#include "detector.h"
#include "decoder.h"
#include "proc.h"
#include "spdifer.h"
#include "dejitter.h"

///////////////////////////////////////////////////////////
// SPDIF status constants

#define SPDIF_MODE_NONE             0  // no data (cannot determine spdif mode)
#define SPDIF_MODE_DISABLED         1  // spdif is disabled
#define SPDIF_MODE_PASSTHROUGH      2  // spdif passthrough
#define SPDIF_MODE_ENCODE           3  // ac3 encode

#define SPDIF_ERR_STEREO_PCM        4  // do not encode stereo pcm
#define SPDIF_ERR_FORMAT            5  // format is not allowed for passthrough
#define SPDIF_ERR_SAMPLE_RATE       6  // sample rate is restricted
#define SPDIF_ERR_SINK              7  // sink refuses spdif output
#define SPDIF_ERR_ENCODER_DISABLED  8  // ac3 encode mode is disabled
#define SPDIF_ERR_PROC              9  // cannot determine encoder format
#define SPDIF_ERR_ENCODER           10 // encoder cannot encode given format


class Spdif2PCM : public NullFilter
{
public:
  Spdif2PCM();
  virtual Speakers get_output() const;
  virtual bool get_chunk(Chunk *_chunk);
};

class DVDGraph : public FilterGraph
{
public:
  Demux          demux;
  Detector       detector;
  Despdifer      despdifer;
  Spdifer        spdifer_pt;
  AudioDecoder   dec;
  AudioProcessor proc;
  AC3Enc         enc;
  Spdifer        spdifer_enc;
  Spdif2PCM      spdif2pcm;
  Syncer         syncer;

public:
  DVDGraph(int nsamples = 4096, const Sink *sink = 0);

  /////////////////////////////////////////////////////////////////////////////
  // DVDGraph interface

  // User format
  bool query_user(Speakers user_spk) const;
  bool set_user(Speakers user_spk);
  Speakers get_user() const;

  // Sink
  void set_sink(const Sink *sink);
  const Sink *get_sink() const;

  bool get_query_sink() const;
  void set_query_sink(bool query_sink);

  // Detector usage

  bool get_use_detector() const;
  void set_use_detector(bool use_detector);

  // SPDIF options
  void set_spdif(bool use_spdif, int spdif_pt, bool spdif_as_pcm, bool spdif_encode, bool spdif_stereo_pt);

  bool get_use_spdif() const;
  void set_use_spdif(bool use_spdif);

  int  get_spdif_pt() const;
  void set_spdif_pt(int spdif_pt);

  bool get_spdif_as_pcm() const;
  void set_spdif_as_pcm(bool spdif_as_pcm);

  bool get_spdif_encode() const;
  void set_spdif_encode(bool spdif_encode);

  bool get_spdif_stereo_pt() const;
  void set_spdif_stereo_pt(bool spdif_stereo_pt);

  int  get_spdif_bitrate() const;
  void set_spdif_bitrate(int bitrate);

  // SPDIF sample rate check
  void set_spdif_sr(bool spdif_check_sr, bool spdif_allow_48, bool spdif_allow_44, bool spdif_allow_32);

  bool get_spdif_check_sr() const;
  void set_spdif_check_sr(bool spdif_check_sr);

  bool get_spdif_allow_48() const;
  void set_spdif_allow_48(bool spdif_allow_48);

  bool get_spdif_allow_44() const;
  void set_spdif_allow_44(bool spdif_allow_44);

  bool get_spdif_allow_32() const;
  void set_spdif_allow_32(bool spdif_allow_32);

  // SPDIF/DTS mode/conversion
  int  get_dts_mode() const;
  void set_dts_mode(int dts_mode);
  int  get_dts_conv() const;
  void set_dts_conv(int dts_conv);

  // SPDIF status
  int get_spdif_status() const;

  // Summary information
  size_t get_info(char *_buf, size_t _len) const;

  /////////////////////////////////////////////////////////////////////////////
  // Filter overrides

  virtual void reset();

protected:
  Speakers user_spk;

  bool     use_detector;

  bool     use_spdif;
  int      spdif_pt;
  bool     spdif_as_pcm;
  bool     spdif_encode;
  bool     spdif_stereo_pt;
  int      spdif_bitrate;

  bool     spdif_check_sr;
  bool     spdif_allow_48;
  bool     spdif_allow_44;
  bool     spdif_allow_32;

  int      spdif_status;
  mutable int spdif_err;

  const Sink *sink;
  bool query_sink;

  enum state_t 
  { 
    state_demux = 0,
    state_detector,
    state_despdif,
    state_spdif_pt,
    state_decode,
    state_proc,
    state_proc_enc,
    state_encode,
    state_spdif_enc, 
    state_spdif2pcm,
    state_dejitter
  };            


  /////////////////////////////////////////////////////////////////////////////
  // FilterGraph overrides

  virtual const char *get_name(int node) const;
  virtual Filter *init_filter(int node, Speakers spk);
  virtual int get_next(int node, Speakers spk) const;

  // helper functions
  int check_spdif_passthrough(Speakers spk) const;
  int check_spdif_encode(Speakers spk) const;
  Speakers agree_output_pcm(Speakers spk, Speakers user_spk) const;
};

#endif
