#include <AudioFilter/BitStream.h>
#include <AudioFilter/SpdifFrameParser.h>

namespace {

AudioFilter::Ac3HeaderParser ac3HeaderParser;
AudioFilter::MpaHeaderParser mpaHeaderParser;
AudioFilter::DtsHdHeaderParser dtsHdHeaderParser;
AudioFilter::DtsHeaderParser dtsHeaderParser;

inline AudioFilter::HeaderParser *findParser(int spdif_type)
{
  switch ( spdif_type )
  {
    // AC3
    case 1:
      return &ac3HeaderParser;
    // MPA
    case 4:
    case 5:
    case 8:
    case 9:
      return &mpaHeaderParser;
    // DTS
    case 11:
    case 12:
    case 13:
      return &dtsHeaderParser;
    case 20:
      return &dtsHdHeaderParser;
    // Unknown
    default:
      return 0;
  }
}

inline const int toBigEndian(int bs_type)
{
  switch ( bs_type )
  {
    case BITSTREAM_16LE:
      return BITSTREAM_16BE;
    case BITSTREAM_32LE:
      return BITSTREAM_32BE;
    case BITSTREAM_14LE:
      return BITSTREAM_14BE;
    default:
      return bs_type;
  }
}

}; // anonymous namespace

namespace AudioFilter {

SpdifFrameParser::SpdifFrameParser(bool isBigEndian)
  : _spdifHeaderParser()
  , _isBigEndian(isBigEndian)
{
  reset();
}

SpdifFrameParser::~SpdifFrameParser()
{}

HeaderParser *SpdifFrameParser::getHeaderParser(void)
{
  return &_spdifHeaderParser;
}

void SpdifFrameParser::reset(void)
{
  _data.ptr = 0;
  _data.size = 0;

  _hi.drop();
}

bool SpdifFrameParser::parseFrame(uint8_t *frame, size_t size)
{
  if ( (frame[0] != 0x00) || (frame[1] != 0x00)
    || (frame[2]  != 0x00) || (frame[3]  != 0x00)
    || (frame[4] != 0x00) || (frame[5] != 0x00)
    || (frame[6]  != 0x00) || (frame[7]  != 0x00)
    || (frame[8] != 0x72) || (frame[9] != 0xf8)
    || (frame[10] != 0x1f) || (frame[11] != 0x4e) )
    return false;

  const SpdifHeaderSync0 *header((SpdifHeaderSync0 *)frame);
  uint8_t *subheader(frame + sizeof(SpdifHeaderSync0));

  HeaderParser *parser(findParser(header->_sync._type));

  if ( parser && parser->parseHeader(subheader, &_hi) )
  {
    _data.ptr = subheader;
    _data.size = header->_sync._len / 8;

    if ( _isBigEndian )
    {
      _hi.bs_type = toBigEndian(_hi.bs_type);
      bs_conv_swab16(_data.ptr, _data.size, _data.ptr);
    }

    return true;
  }

  _hi.drop();
  return false;
}

std::string SpdifFrameParser::getStreamInfo(void) const
{
  return "";
}

std::string SpdifFrameParser::getFrameInfo(void) const
{
  return "";
}

}; // namespace AudioFilter

// vim: ts=2 sts=2 et

