// remove these when done debuggin
//#include <cstdlib>
//#include <iostream>
//#include <cstdio>
//#include <cstring>
//#ifdef __unix__
//#ifndef _XOPEN_SOURCE
//#define _XOPEN_SOURCE
//#endif
//#include <unistd.h>
//#else
//#endif

#include <AudioFilter/Parsers.h>
#include <AudioFilter/BitReader.h>

namespace {

}; // anonymous namespace

namespace AudioFilter {

const uint32_t dca_bit_rates[36] =
{
    32000, 56000, 64000, 96000, 112000, 128000,
    192000, 224000, 256000, 320000, 384000,
    448000, 512000, 576000, 640000, 768000,
    896000, 1024000, 1152000, 1280000, 1344000,
    1408000, 1411200, 1472000, 1536000, 1920000,
    2048000, 3072000, 3840000, 1, 2, 3
};

class DtsFrameInfo
{
public:
  DtsFrameInfo()
    : _isNormalFrame(false)
    , _shortSamples(0)
    , _isCrcPresent(false)
    , _sampleBlocks(0)
    , _frameSize(0)
    , _channelLayout(0)
    , _sampleIndex(0)
    , _bitRate(0)
    , _downMix(false)
    , _dynamicRange(false)
    , _timeStamp(false)
    , _auxiliaryData(false)
    , _hdcd(false)
    , _externalDescription(0)
    , _externalCoding(false)
    , _aspf(false)
    , _lfe(0)
    , _predictorHistory(false)
    , _crc(0)
    , _multirateInter(false)
    , _version(0)
    , _copyHistory(0)
    , _sourcePcmResolution(0)
    , _frontSum(false)
    , _surroundSum(false)
    , _dialogNormalization(0)
    , _is14Bit(false)
  {
  }

  bool init ( BitReader br )
  {
    br.reset();
    // 16 bit formats
    if ( br.getByte() == 0x7f
            && br.getByte() == 0xfe
            && br.getByte() == 0x80
            && br.getByte() == 0x01 )
    {
        _is14Bit = false;
    }
    else
    {
      // 14 bit formats
      br.reset();
      if (br.getByte() == 0x1f
        && br.getByte() == 0xff
        && br.getByte() == 0xe8
        && br.getByte() == 0x00
        && br.getByte() == 0x07
        && br.getByte(4) == 0xf)
      {
        _is14Bit = true;
/*
 * The original code was this, not sure I can parse the 14bit the same as the
 * 16 bit, but I can look at that later if there are problems
    sampleBlocks = ((be2uint16(hdr16[2]) << 4)  & 0x70)
            | ((be2uint16(hdr16[3]) >> 10) & 0x0f);
    ++sampleBlocks;
    amode = (be2uint16(hdr16[4]) >> 4)  & 0x3f;
    sfreq = (be2uint16(hdr16[4]) >> 0)  & 0x0f;
    lfe   = (be2uint16(hdr16[6]) >> 11) & 0x03;
*/
      }
      else
      {
        return false; // invalid frame data
      }
    }

    _isNormalFrame = br.getBool(); // 1
    _shortSamples = br.getInt(5); // 6
    _isCrcPresent = br.getBool(); // 7
    _sampleBlocks = br.getInt(7) + 1; // 14
    _frameSize = br.getInt(14) + 1; // 28
    _channelLayout = br.getInt(6);
    _sampleIndex = br.getInt(4);
    _bitRate = br.getInt(5);
    _downMix = br.getBool();
    _dynamicRange = br.getBool();
    _timeStamp = br.getBool();
    _auxiliaryData = br.getBool();
    _hdcd = br.getBool();
    _externalDescription = br.getInt(3);
    _externalCoding = br.getBool();
    _aspf = br.getBool();
    _lfe = br.getInt(2);
    _predictorHistory = br.getBool();

    if ( _isCrcPresent )
      _crc = br.getInt(16);

    _multirateInter = br.getBool();
    _version = br.getInt(4);
    _copyHistory = br.getInt(2);
    _sourcePcmResolution = br.getInt(3);
    _frontSum = br.getBool();
    _surroundSum = br.getBool();
    _dialogNormalization = br.getInt(4);
    return true;
  }

  int getSampleCount(void) const
  {
      return _sampleBlocks * 32;
  }

  int getSampleBlocks(void) const
  {
      return _sampleBlocks;
  }

  int getFrameSize(void) const
  {
      return _frameSize;
  }

  int getChannelLayout(void) const
  {
    return _channelLayout;
  }

  int getSampleIndex(void) const
  {
    return _sampleIndex;
  }

  int getSampleRate(void) const
  {
    const int rates[] = { 0, 8000, 16000, 32000, 0, 0 , 11025, 22050
            , 44100, 0, 0, 12000, 24000, 48000, 96000, 192000 };

    return rates[_sampleIndex];
  }

  int getLfe(void) const
  {
      return _lfe;
  }

  bool getIs14Bit(void) const
  {
      return _is14Bit;
  }

private:
  bool _isNormalFrame;
  int _shortSamples;
  bool _isCrcPresent;
  int _sampleBlocks;
  int _frameSize;
  int _channelLayout;
  int _sampleIndex;
  int _bitRate;
  bool _downMix;
  bool _dynamicRange;
  bool _timeStamp;
  bool _auxiliaryData;
  bool _hdcd;
  int _externalDescription;
  bool _externalCoding;
  bool _aspf;
  int _lfe;
  bool _predictorHistory;
  int _crc;
  bool _multirateInter;
  int _version;
  int _copyHistory;
  int _sourcePcmResolution;
  bool _frontSum;
  bool _surroundSum;
  int _dialogNormalization;
  bool _is14Bit;
};

class DtsHdFrameInfo
{
public:
  DtsHdFrameInfo()
    : _ssIndex(0)
    , _headerSize(0)
    , _hdSize(0)
  {
  }

  DtsHdFrameInfo(BitReader br)
    : _ssIndex(0)
    , _headerSize(0)
    , _hdSize(0)
  {
    init(br);
  }

  bool init ( BitReader br )
  {
    br.reset();
    if ( br.getByte() != 0x64
        || br.getByte() != 0x58
        || br.getByte() != 0x20
        || br.getByte() != 0x25 )
    {
        return false;
    }

    // * dca.c dca_exss_parse_header

    br.skip(8); // user data
    _ssIndex = br.getInt(2);
    bool blownUp(br.getBool());
    _headerSize = br.getInt(blownUp ? 12 : 8);
    //hdLen = ((hdHdr[6] & 0xf) << 11) + (hdHdr[7] << 3) + ((hdHdr[8] >> 5) & 7) + 1;
    _hdSize = br.getInt(blownUp ? 20: 16) + 1;

    return true;
  }

  int getHdSize(void) const
  {
      return _hdSize;
  }

private:
  int _ssIndex;
  int _headerSize;
  int _hdSize;
};

bool DtsHeaderParser::parseHeader(const uint8_t *hdr, HeaderInfo *hinfo)
{
  DtsFrameInfo dtsFi;
  int bsType;
  bool isBigEndian(true);

  if ( dtsFi.init(BitReader(hdr, 16, true)) )
      bsType = dtsFi.getIs14Bit() ? BITSTREAM_14BE : BITSTREAM_16BE;
  else if ( dtsFi.init(BitReader(hdr, 16, false)) )
  {
      bsType = dtsFi.getIs14Bit() ? BITSTREAM_14LE : BITSTREAM_16LE;
      isBigEndian = false;
  }
  else
    return false;

  /////////////////////////////////////////////////////////
  // Constraints

  if ( dtsFi.getSampleBlocks() < 6 ) // constraint
    return false;
  else if ( dtsFi.getChannelLayout() > 0xc ) // we don't work with more than 6 channels
    return false;
  else if ( dtsFi.getSampleRate() == 0 ) // constraint
    return false;
  else if ( dtsFi.getLfe() == 3 ) // constraint
    return false;

  DtsHdFrameInfo dtsHdFrameInfo(BitReader(hdr+dtsFi.getFrameSize(), 16, isBigEndian));

  if ( hinfo )
  {
    const int layout2MaskTable[] = {
      MODE_MONO,   MODE_STEREO,  MODE_STEREO,  MODE_STEREO,  MODE_STEREO,
      MODE_3_0,    MODE_2_1,     MODE_3_1,     MODE_2_2,     MODE_3_2
    };

    const int layout2RelationTable[] =
    {
      NO_RELATION,   NO_RELATION,  NO_RELATION,  RELATION_SUMDIFF, RELATION_DOLBY,
      NO_RELATION,   NO_RELATION,  NO_RELATION,  NO_RELATION,      NO_RELATION,
    };

    int mask(layout2MaskTable[dtsFi.getChannelLayout()]);
    const int relation(layout2RelationTable[dtsFi.getChannelLayout()]);

    if ( dtsFi.getLfe() )
    {
      mask |= CH_MASK_LFE;
    }

    hinfo->setSpeakers(Speakers(FORMAT_DTS, mask, dtsFi.getSampleRate(), 1.0, relation));
    hinfo->setFrameSize(dtsFi.getFrameSize() + dtsHdFrameInfo.getHdSize());
    hinfo->setDtsHdSize(dtsHdFrameInfo.getHdSize());
    hinfo->setScanSize(16384); // always scan up to maximum DTS frame size
    hinfo->setSampleCount(dtsFi.getSampleCount());
    hinfo->setBsType(bsType);

    switch ( hinfo->getSampleCount() )
    {
      case 512:
        hinfo->setSpdifType(11);
        break;
      case 1024:
        hinfo->setSpdifType(12);
        break;
      case 2048:
        hinfo->setSpdifType(13);
        break;
      default:
        hinfo->setSpdifType(0); // cannot do SPDIF passthrough
        break;
    }
  }

  return true;
}

bool DtsHeaderParser::compareHeaders(const uint8_t *hdr1, const uint8_t *hdr2)
{
  // Compare only:
  // * syncword
  // * AMODE
  // * SFREQ
  // * LFF (do not compare interpolation type)

  // 16 bits big endian bitstream
  if (hdr1[0] == 0x7f
        && hdr1[1] == 0xfe
        && hdr1[2] == 0x80
        && hdr1[3] == 0x01)
  {
    return
      hdr1[0] == hdr2[0] && hdr1[1] == hdr2[1] && // syncword
      hdr1[2] == hdr2[2] && hdr1[3] == hdr2[3] && // syncword
      (hdr1[7] & 0x0f) == (hdr2[7] & 0x0f) &&     // AMODE
      (hdr1[8] & 0xfc) == (hdr2[8] & 0xfc) &&     // AMODE, SFREQ
      ((hdr1[10] & 0x06) != 0) == ((hdr2[10] & 0x06) != 0); // LFF
  }
  // 16 bits little endian bitstream
  else if (hdr1[0] == 0xfe
        && hdr1[1] == 0x7f
        && hdr1[2] == 0x01
        && hdr1[3] == 0x80)
  {
    return
      hdr1[0] == hdr2[0] && hdr1[1] == hdr2[1] && // syncword
      hdr1[2] == hdr2[2] && hdr1[3] == hdr2[3] && // syncword
      (hdr1[6] & 0x0f) == (hdr2[6] & 0x0f) &&     // AMODE
      (hdr1[9] & 0xfc) == (hdr2[9] & 0xfc) &&     // AMODE, SFREQ

      ((hdr1[11] & 0x06) != 0) == ((hdr2[11] & 0x06) != 0); // LFF
  }
  // 14 bits big endian bitstream
  else if (hdr1[0] == 0x1f
        && hdr1[1] == 0xff
        && hdr1[2] == 0xe8
        && hdr1[3] == 0x00
        && (hdr1[4] & 0xfc) == 0x04)
  {
    return
      hdr1[0] == hdr2[0] && hdr1[1] == hdr2[1] && // syncword
      hdr1[2] == hdr2[2] && hdr1[3] == hdr2[3] && // syncword
      (hdr1[4] & 0xfc) == (hdr2[4] & 0xfc) &&     // syncword
      (hdr1[8] & 0x03) == (hdr2[8] & 0x03) &&     // AMODE
      hdr1[9] == hdr2[9] &&                       // AMODE, SFREQ
      ((hdr1[12] & 0x18) != 0) == ((hdr2[12] & 0x18) != 0); // LFF
  }
  // 14 bits little endian bitstream
  else if (hdr1[0] == 0xff
        && hdr1[1] == 0x1f
        && hdr1[2] == 0x00
        && hdr1[3] == 0xe8
        && (hdr1[5] & 0xfc) == 0x04)
  {
    return
      hdr1[0] == hdr2[0] && hdr1[1] == hdr2[1] && // syncword
      hdr1[2] == hdr2[2] && hdr1[3] == hdr2[3] && // syncword
      (hdr1[5] & 0xfc) == (hdr2[5] & 0xfc) &&     // syncword
      (hdr1[9] & 0x03) == (hdr2[9] & 0x03) &&     // AMODE
      hdr1[8] == hdr2[8] &&                       // AMODE, SFREQ
      ((hdr1[13] & 0x18) != 0) == ((hdr2[13] & 0x18) != 0); // LFF
  }
  // no sync
  else
    return false;
}

}; // namespace AudioFilter

// vim: ts=2 sts=2 et

