#include <cstdio>
#include <AudioFilter/Parsers.h>

namespace {

AudioFilter::Ac3HeaderParser ac3HeaderParser;
AudioFilter::MpaHeaderParser mpaHeaderParser;
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
    // Unknown
    default:
      return 0;
  }
}

}; // anonymous namespace

namespace AudioFilter {

bool SpdifHeaderParser::parseHeader(const uint8_t *hdr, HeaderInfo *hinfo)
{
  if ( (hdr[0] != 0x00) || (hdr[1] != 0x00)
      || (hdr[2]  != 0x00) || (hdr[3]  != 0x00)
      || (hdr[4] != 0x00) || (hdr[5] != 0x00)
      || (hdr[6]  != 0x00) || (hdr[7]  != 0x00)
      || (hdr[8] != 0x72) || (hdr[9] != 0xf8)
      || (hdr[10] != 0x1f) || (hdr[11] != 0x4e) )
    return false;

  const SpdifHeaderSync0 *header((SpdifHeaderSync0 *)hdr);
  HeaderParser *parser(findParser(header->_sync._type));

  if ( parser )
  {
    const uint8_t *subheader(hdr + sizeof(SpdifHeaderSync0));
    HeaderInfo subinfo;

    if ( parser->parseHeader(subheader, &subinfo) )
    {
      // SPDIF frame size equals to number of samples in contained frame
      // multiplied by 4 (see Spdifer class for more details)

      if ( hinfo )
      {
        hinfo->bs_type = BITSTREAM_16LE;
        hinfo->spk = subinfo.spk;
        hinfo->spk.format = FORMAT_SPDIF;
        hinfo->frame_size = subinfo.nsamples * 4;
        hinfo->scan_size = subinfo.nsamples * 4;
        hinfo->nsamples = subinfo.nsamples;
        hinfo->spdif_type = header->_sync._type;
      }

      return true;
    }
  }

  return false;
}

bool SpdifHeaderParser::compareHeaders(const uint8_t *hdr1, const uint8_t *hdr2)
{
  if ( (hdr1[0] != 0x00) || (hdr1[1] != 0x00)
      || (hdr1[2]  != 0x00) || (hdr1[3]  != 0x00)
      || (hdr1[4] != 0x00) || (hdr1[5] != 0x00)
      || (hdr1[6]  != 0x00) || (hdr1[7]  != 0x00)
      || (hdr1[8] != 0x72) || (hdr1[9] != 0xf8)
      || (hdr1[10] != 0x1f) || (hdr1[11] != 0x4e)
      || (hdr2[0] != 0x00) || (hdr2[1] != 0x00)
      || (hdr2[2]  != 0x00) || (hdr2[3]  != 0x00)
      || (hdr2[4] != 0x00) || (hdr2[5] != 0x00)
      || (hdr2[6]  != 0x00) || (hdr2[7]  != 0x00)
      || (hdr2[8] != 0x72) || (hdr2[9] != 0xf8)
      || (hdr2[10] != 0x1f) || (hdr2[11] != 0x4e) )

    return false;

  const SpdifHeaderSync0 *header((SpdifHeaderSync0 *)hdr1);
  HeaderParser *parser(findParser(header->_sync._type));

  if ( parser )
    return parser->compareHeaders(hdr1 + sizeof(SpdifHeaderSync0), hdr2 + sizeof(SpdifHeaderSync0));

  return false;
}

std::string SpdifHeaderParser::getHeaderInfo(const uint8_t *hdr)
{
  char info[1024];
  size_t info_size = 0;
  info[0] = 0;

  if ( (hdr[0] == 0x00) && (hdr[1] == 0x00)
      && (hdr[2]  == 0x00) && (hdr[3]  == 0x00)
      && (hdr[4] == 0x00) && (hdr[5] == 0x00)
      && (hdr[6]  == 0x00) && (hdr[7]  == 0x00)
      && (hdr[8] == 0x72) && (hdr[9] == 0xf8)
      && (hdr[10] == 0x1f) && (hdr[11] == 0x4e) )
  {
    SpdifHeaderSync0 *header((SpdifHeaderSync0 *)hdr);
    HeaderParser *parser(findParser(header->_sync._type));

    if ( parser )
    {
      const uint8_t *subheader(hdr + sizeof(SpdifHeaderSync0));
      HeaderInfo subinfo;

      if ( parser->parseHeader(subheader, &subinfo) )
      {
        info_size += sprintf(info + info_size, "Stream format: SPDIF/%s %s %iHz\n",
			     subinfo.spk.getFormatText(), subinfo.spk.getModeText(),
			     subinfo.spk.sample_rate);
        const std::string &hi = parser->getHeaderInfo(subheader);
        strcat(info, hi.c_str());
        info_size += hi.size();
      }
      else
        info_size += sprintf(info + info_size, "Cannot parse substream header\n");
    }
    else
      info_size += sprintf(info + info_size, "Unknown substream\n");
  }
  else
    info_size += sprintf(info + info_size, "No SPDIF header found\n");

  return info;
}

}; // namespace AudioFilter

// vim: ts=2 sts=2 et

