#include <cstdio>
#include <AudioFilter/BitStream.h>
#include <AudioFilter/SpdifWrapper.h>

namespace {

inline bool is14Bit(int bsType)
{
  return (bsType == BITSTREAM_14LE) || (bsType == BITSTREAM_14BE);
}

const size_t MAX_SPDIF_FRAME_SIZE(16384*4);

class DtsHdHeader
{
public:
  DtsHdHeader()
  {
    //_c[0] = 0x01;
    _c[0] = 0x00;
    //_c[1] = 0x00;
    _c[1] = 0x01;
    _c[2] = 0x00;
    _c[3] = 0x00;
    _c[4] = 0x00;
    _c[5] = 0x00;
    _c[6] = 0x00;
    _c[7] = 0x00;
    _c[8] = 0xfe;
    _c[9] = 0xfe;
    _size = 0;
  }

  void setSize(uint16_t size)
  {
    //_c[0] = 0x01;
    _c[0] = 0x00;
    //_c[1] = 0x00;
    _c[1] = 0x01;
    _c[2] = 0x00;
    _c[3] = 0x00;
    _c[4] = 0x00;
    _c[5] = 0x00;
    _c[6] = 0x00;
    _c[7] = 0x00;
    _c[8] = 0xfe;
    _c[9] = 0xfe;
    _size = size;
  }
  uint8_t _c[10];
  uint16_t _size;
};

//const char DTSHD_START_CODE[10] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xfe };

}; // anonymous namespace

namespace AudioFilter {

SpdifWrapper::SpdifWrapper(HeaderParser *ph
          , int dtsMode, int dtsConv)
  : _dtsMode(dtsMode)
  , _dtsConv(dtsConv)
  , _buf(new uint8_t[MAX_SPDIF_FRAME_SIZE])
  , _pHeaderParser(ph)
  , _isDefaultHeaderParser(!ph)
  , _spdifFrameParser(false)
  , _hi()
  , _spk()
  , _spdifFrame()
  , _useHeader(true)
  , _spdifBsType(0)
  , _channelMap()
  , _clearOffsetLast(0)
{
  ::memset(_buf, 0, MAX_SPDIF_FRAME_SIZE);

  if ( ! ph )
  {
    MultiHeaderParser *mhp(new MultiHeaderParser());
    mhp->addParser(new SpdifHeaderParser());
    mhp->addParser(new DtsHeaderParser());
    mhp->addParser(new Ac3HeaderParser());
    mhp->addParser(new MpaHeaderParser());
    _pHeaderParser = mhp;
  }
}

SpdifWrapper::~SpdifWrapper()
{
  delete[] _buf;

  if ( _isDefaultHeaderParser && _pHeaderParser )
  {
    delete _pHeaderParser;
  }
}

///////////////////////////////////////////////////////////////////////////////
// FrameParser overrides

HeaderParser *SpdifWrapper::getHeaderParser(void)
{
  return _pHeaderParser;
}

void SpdifWrapper::reset(void)
{
  _hi.drop();
  _spdifFrameParser.reset();
  _spk = Speakers::UNKNOWN;
  _spdifFrame.ptr = 0;
  _spdifFrame.size = 0;
}

bool SpdifWrapper::parseFrame(uint8_t *frame, size_t size)
{
  _spk = Speakers(FORMAT_SPDIF, MODE_STEREO, 48000, 1.0, NO_RELATION); // set a plausible default

  if ( ! _pHeaderParser->parseHeader(frame, &_hi) ) // unknown format
  {
    return false;
    // don't reset _spdifFrame.ptr and _spdifFrame.size
    // we may want to resend the last valid frame
    // in the event a bogus frame is encountered
  }

  _spdifFrame.ptr = 0;
  _spdifFrame.size = 0;

  if ( ! _hi.isSpdifable() ) // passthrough non-spdifable format
  {
    _spk = _hi.getSpeakers();
    _spdifFrame.ptr = frame;
    _spdifFrame.size = size;
    return true;
  }

  /////////////////////////////////////////////////////////
  // Parse spdif input if necessary

  uint8_t *rawFrame(frame);
  size_t rawSize(size);

  if ( _hi.getSpeakers().isSpdif() ) // if it's already spdif'd
  {
    if ( ! _spdifFrameParser.parseFrame(frame, size) )
      return false;

    _hi = _spdifFrameParser.getHeaderInfo();
    rawFrame = _spdifFrameParser.getRawData();
    rawSize = _spdifFrameParser.getRawSize();
  }

  size_t spdifFrameSize(_hi.getSampleCount() * 4);
  size_t maxSpdifFrameSize(2048);
  const bool isDtsHd(_hi.isDtsHd());
  size_t spdifHeaderSize(sizeof(SpdifHeaderSync0));
  size_t hdFrameMultiplier(1);
  bool hasDtsHdSupport( isValidChannelMap(8, 192000) \
                  || isValidChannelMap(2, 192000) \
                  || isValidChannelMap(8, 48000) );

  if ( isDtsHd )
  {
    if ( hasDtsHdSupport )
    {
      if ( isValidChannelMap(8, 192000) )
      {
        hdFrameMultiplier = 16;
        spdifHeaderSize = (sizeof(SpdifHeaderSync) + sizeof(DtsHdHeader));
      }
      else if ( isValidChannelMap(2, 192000) || isValidChannelMap(8, 48000) )
      {
        hdFrameMultiplier = 4;
        spdifHeaderSize = (sizeof(SpdifHeaderSync) + sizeof(DtsHdHeader));
      }

      spdifFrameSize *= hdFrameMultiplier;
      maxSpdifFrameSize *= hdFrameMultiplier;
    }
    else
    {
      // ignore the dts hd data when we don't have enough bandwidth
      rawSize -= _hi.getDtsHdSize();
    }
  }

  const size_t spdifDataSize(spdifFrameSize - spdifHeaderSize);

  if ( rawSize > spdifDataSize )
  {
    rawSize = spdifDataSize;
  }

  if ( spdifFrameSize > MAX_SPDIF_FRAME_SIZE )
  {
    // impossible to send over spdif
    // passthrough non-spdifable data
    _spk = _hi.getSpeakers();
    _spdifFrame.ptr = rawFrame;
    _spdifFrame.size = rawSize;
    return true;
  }

  /////////////////////////////////////////////////////////
  // Determine output bitstream type and header usage

  if ( _hi.getSpeakers().isDts() )
  {
    const bool frameGrows((_dtsConv == DTS_CONV_14BIT)
                    && ! is14Bit(_hi.getBsType()));
    const bool frameShrinks((_dtsConv == DTS_CONV_16BIT)
                    && is14Bit(_hi.getBsType()));

    switch ( _dtsMode )
    {
      case DTS_MODE_WRAPPED:
        _useHeader = true;

        if ( frameGrows && (rawSize * 8 / 7 <= spdifDataSize) )
          _spdifBsType = BITSTREAM_14LE;
        else if ( frameShrinks && (rawSize * 7 / 8 <= spdifDataSize) )
          _spdifBsType = BITSTREAM_16LE;
        else if ( rawSize <= spdifDataSize )
          _spdifBsType = ( is14Bit(_hi.getBsType()) ) ? BITSTREAM_14LE : BITSTREAM_16LE;
        else
        {
          // impossible to wrap
          // passthrough non-spdifable data
          _spk = _hi.getSpeakers();
          _spdifFrame.ptr = rawFrame;
          _spdifFrame.size = rawSize;
          return true;
        }
      break;

      case DTS_MODE_PADDED:
        _useHeader = false;

        if ( frameGrows && (rawSize * 8 / 7 <= spdifFrameSize) )
          _spdifBsType = BITSTREAM_14LE;
        else if ( frameShrinks && (rawSize * 7 / 8 <= spdifFrameSize) )
          _spdifBsType = BITSTREAM_16LE;
        else if ( rawSize <= spdifFrameSize )
          _spdifBsType = ( is14Bit(_hi.getBsType()) ) ? BITSTREAM_14LE : BITSTREAM_16LE;
        else
        {
          // impossible to send over spdif
          // passthrough non-spdifable data
          _spk = _hi.getSpeakers();
          _spdifFrame.ptr = rawFrame;
          _spdifFrame.size = rawSize;
          return true;
        }
        break;

      case DTS_MODE_AUTO:
      default:
        if ( frameGrows && (rawSize * 8 / 7 <= spdifDataSize) )
        {
          _useHeader = true;
          _spdifBsType = BITSTREAM_14LE;
        }
        else if ( frameGrows && (rawSize * 8 / 7 <= spdifFrameSize) )
        {
          _useHeader = false;
          _spdifBsType = BITSTREAM_14LE;
        }

        if ( frameShrinks && (rawSize * 7 / 8 <= spdifDataSize) )
        {
          _useHeader = true;
          _spdifBsType = BITSTREAM_14LE;
        }
        else if ( frameShrinks && (rawSize * 7 / 8 <= spdifFrameSize) )
        {
          _useHeader = false;
          _spdifBsType = BITSTREAM_14LE;
        }
        else if ( rawSize <= spdifDataSize )
        {
          _useHeader = true;
          _spdifBsType = ( is14Bit(_hi.getBsType()) ) ? BITSTREAM_14LE : BITSTREAM_16LE;
        }
        else if ( rawSize <= spdifFrameSize )
        {
          _useHeader = false;
          _spdifBsType = ( is14Bit(_hi.getBsType()) ) ? BITSTREAM_14LE : BITSTREAM_16LE;
        }
        else
        {
          // impossible to send over spdif
          // passthrough non-spdifable data
          _spk = _hi.getSpeakers();
          _spdifFrame.ptr = rawFrame;
          _spdifFrame.size = rawSize;
          return true;
        }
        break;
    }
  }
  else
  {
    _useHeader = true;
    _spdifBsType = BITSTREAM_16LE;
  }

  /////////////////////////////////////////////////////////
  // Fill payload, convert bitstream type and init header

  size_t payloadSize(0);

  if ( _useHeader )
  {
    if ( isDtsHd && hasDtsHdSupport )
    {
      payloadSize = bs_convert(rawFrame, rawSize, _hi.getBsType()
                      , _buf + spdifHeaderSize, _spdifBsType);
      assert(payloadSize <= spdifDataSize);

      const size_t clearOffset(spdifHeaderSize + payloadSize);
      //const size_t clearCount(spdifDataSize - payloadSize);

      if ( _clearOffsetLast > clearOffset )
        ::memset(_buf + clearOffset, 0, _clearOffsetLast - clearOffset);

      _clearOffsetLast = clearOffset;

      // correct DTS syncword when converting to 14bit
      if ( _spdifBsType == BITSTREAM_14LE )
        _buf[sizeof(SpdifHeaderSync) + 3] = 0xe8;

      SpdifHeaderSync *hs((SpdifHeaderSync *)_buf);
      DtsHdHeader *hd((DtsHdHeader *)(_buf+sizeof(SpdifHeaderSync)));
      hs->set( (hdFrameMultiplier == 16)
                      ? 0x0411
                      : ((hdFrameMultiplier == 8)
                              ? 0x0311
                              : 0x0211)
                      , rawSize + sizeof(DtsHdHeader));
      hd->setSize(rawSize);
    }
    else
    {
      payloadSize = bs_convert(rawFrame, rawSize, _hi.getBsType()
                      , _buf + spdifHeaderSize, _spdifBsType);
      assert(payloadSize <= spdifDataSize);

      const size_t clearOffset(spdifHeaderSize + payloadSize);
      //const size_t clearCount(spdifDataSize - payloadSize);

      if ( _clearOffsetLast > clearOffset )
        ::memset(_buf + clearOffset, 0, _clearOffsetLast - clearOffset);

      _clearOffsetLast = clearOffset;

      // correct DTS syncword when converting to 14bit
      if ( _spdifBsType == BITSTREAM_14LE )
        _buf[sizeof(SpdifHeaderSync0) + 3] = 0xe8;

      SpdifHeaderSync0 *hs((SpdifHeaderSync0 *)_buf);
      hs->set(_hi.getSpdifType(), (uint16_t)payloadSize * 8);
    }
  }
  else
  {
    payloadSize = bs_convert(rawFrame, rawSize, _hi.getBsType()
                    , _buf, _spdifBsType);
    assert(payloadSize <= spdifFrameSize);

      const size_t clearOffset(payloadSize);
      //const size_t clearCount(spdifFrameSize - payloadSize);

      if ( _clearOffsetLast > clearOffset )
        ::memset(_buf + clearOffset, 0, _clearOffsetLast - clearOffset);

      _clearOffsetLast = clearOffset;

    // correct DTS syncword when converting to 14bit
    if ( _spdifBsType == BITSTREAM_14LE )
      _buf[3] = 0xe8;
  }

  if ( ! payloadSize ) // cannot convert bitstream
    return false;

  // prepare output
  _spk = _hi.getSpeakers();

  if ( isDtsHd && hasDtsHdSupport )
  {
    if ( isValidChannelMap(8, 192000) )
    {
      _spk.setMask(MODE_7_1);
      _spk.setSampleRate(192000);
    }
    else if ( isValidChannelMap(2, 192000) )
    {
      _spk.setMask(MODE_STEREO);
      _spk.setSampleRate(192000);
    }
    else if ( isValidChannelMap(8, 48000) )
    {
      _spk.setMask(MODE_7_1);
      _spk.setSampleRate(48000);
    }
    else if ( isValidChannelMap(2, 48000) )
    {
      _spk.setMask(MODE_STEREO);
      _spk.setSampleRate(48000);
    }
  }
  else
  {
    _spk.setMask(MODE_STEREO);
  }

  _spk.setFormat(FORMAT_SPDIF);
  _spdifFrame.ptr = _buf;
  _spdifFrame.size = spdifFrameSize;

  return true;
}

bool SpdifWrapper::isValidChannelMap ( int channels, int sampleRate )
{
  std::vector<ChannelMap>::const_iterator i;

  for ( i = _channelMap.begin(); i != _channelMap.end(); i++ )
  {
    if ( (*i)._channels == channels && (*i)._sampleRate == sampleRate )
      return true;
  }

  return false;
}

std::string SpdifWrapper::getStreamInfo(void) const
{
  char info[1024];
  info[0] = 0;

  if ( ! _spk.isUnknown() )
  {
    const char *bitstream = "???";

    switch ( _spdifBsType )
    {
      case BITSTREAM_16BE:
        bitstream = "16bit BE";
        break;
      case BITSTREAM_16LE:
        bitstream = "16bit LE";
        break;
      case BITSTREAM_14BE:
        bitstream = "14bit BE";
        break;
      case BITSTREAM_14LE:
        bitstream = "14bit LE";
        break;
    }

    sprintf(info,
      "Output format: %s %s %iHz\n"
      "SPDIF format: %s\n"
      "Bitstream: %s\n"
      "Frame size: %zu\n",
      _spk.getFormatText(),
      _spk.getModeText(),
      _spk.getSampleRate(),
      _useHeader ? "wrapped": "padded",
      bitstream,
      _spdifFrame.size );
  }

  return info;
}

std::string SpdifWrapper::getFrameInfo(void) const
{
  return "";
}

}; // namespace AudioFilter

// vim: ts=2 sts=2 et

