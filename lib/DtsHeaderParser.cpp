// remove these when done debuggin
#include <cstdlib>
#include <iostream>
#include <cstdio>

#include <AudioFilter/Parsers.h>

namespace {

const int dts_sample_rates[] =
{
  0, 8000, 16000, 32000, 0, 0, 11025, 22050, 44100, 0, 0,
  12000, 24000, 48000, 96000, 192000
};

const int amode2mask_tbl[] =
{
  MODE_MONO,   MODE_STEREO,  MODE_STEREO,  MODE_STEREO,  MODE_STEREO,
  MODE_3_0,    MODE_2_1,     MODE_3_1,     MODE_2_2,     MODE_3_2
};

const int amode2rel_tbl[] =
{
  NO_RELATION,   NO_RELATION,  NO_RELATION,  RELATION_SUMDIFF, RELATION_DOLBY,
  NO_RELATION,   NO_RELATION,  NO_RELATION,  NO_RELATION,      NO_RELATION,
};

}; // anonymous namespace

namespace AudioFilter {

bool DtsHeaderParser::parseHeader(const uint8_t *hdr, HeaderInfo *hinfo)
{
  int bs_type;
  int nblks;
  int amode;
  int sfreq;
  int lff;
  int fSiz(0);
  int hdLen(0);
  uint16_t *hdr16((uint16_t *)hdr);

  //uint32_t syncword_dts = AV_RB32(pkt->data);

  // 16 bits big endian bitstream
  if ( hdr[0] == 0x7f
        && hdr[1] == 0xfe
        && hdr[2] == 0x80
        && hdr[3] == 0x01 )
  {
    bs_type = BITSTREAM_16BE;
    nblks = (be2uint16(hdr16[2]) >> 2)  & 0x7f;
    fSiz = ((((int16_t)hdr[5] << 14) & 0x3fff)
            | ((int16_t)hdr[6] << 4)
            | ((int16_t)hdr[7] >> 4)) + 1;
    amode = ((be2uint16(hdr16[3]) << 2)  & 0x3c)
            | ((be2uint16(hdr16[4]) >> 14) & 0x03);
    sfreq = (be2uint16(hdr16[4]) >> 10) & 0x0f;
    lff   = (be2uint16(hdr16[5]) >> 9)  & 0x03;
    ++nblks;

    const uint8_t *hdHdr(hdr+fSiz);

    if ( hdHdr[0] == 0x64
        && hdHdr[1] == 0x58
        && hdHdr[2] == 0x20
        && hdHdr[3] == 0x25 )
    {
      hdLen = ((hdHdr[6] & 0xf) << 11) + (hdHdr[7] << 3) + ((hdHdr[8] >> 5) & 7) + 1;
    }

  }
  // 16 bits little endian bitstream
  else if (hdr[0] == 0xfe
        && hdr[1] == 0x7f
        && hdr[2] == 0x01
        && hdr[3] == 0x80)
  {
    bs_type = BITSTREAM_16LE;
    nblks = (le2uint16(hdr16[2]) >> 2)  & 0x7f;
    amode = ((le2uint16(hdr16[3]) << 2)  & 0x3c)
            | ((le2uint16(hdr16[4]) >> 14) & 0x03);
    sfreq = (le2uint16(hdr16[4]) >> 10) & 0x0f;
    lff   = (le2uint16(hdr16[5]) >> 9)  & 0x03;
    ++nblks;
  }
  // 14 bits big endian bitstream
  else if (hdr[0] == 0x1f
        && hdr[1] == 0xff
        && hdr[2] == 0xe8
        && hdr[3] == 0x00
        && hdr[4] == 0x07
        && ((hdr[5] & 0xf0) == 0xf0))
  {
    bs_type = BITSTREAM_14BE;
    nblks = ((be2uint16(hdr16[2]) << 4)  & 0x70)
            | ((be2uint16(hdr16[3]) >> 10) & 0x0f);
    amode = (be2uint16(hdr16[4]) >> 4)  & 0x3f;
    sfreq = (be2uint16(hdr16[4]) >> 0)  & 0x0f;
    lff   = (be2uint16(hdr16[6]) >> 11) & 0x03;
    ++nblks;
  }
  // 14 bits little endian bitstream
  else if (hdr[0] == 0xff
        && hdr[1] == 0x1f
        && hdr[2] == 0x00
        && hdr[3] == 0xe8
        && ((hdr[4] & 0xf0) == 0xf0)
        && hdr[5] == 0x07)
  {
    bs_type = BITSTREAM_14LE;
    nblks = ((le2uint16(hdr16[2]) << 4)  & 0x70)
            | ((le2uint16(hdr16[3]) >> 10) & 0x0f);
    amode = (le2uint16(hdr16[4]) >> 4)  & 0x3f;
    sfreq = (le2uint16(hdr16[4]) >> 0)  & 0x0f;
    lff   = (le2uint16(hdr16[6]) >> 11) & 0x03;
    ++nblks;
  }
  // no sync
  else
    return false;

  /////////////////////////////////////////////////////////
  // Constraints

  if ( nblks < 6 ) // constraint
    return false;
  else if ( amode > 0xc ) // we don't work with more than 6 channels
    return false;
  else if ( dts_sample_rates[sfreq] == 0 ) // constraint
    return false;
  else if ( lff == 3 ) // constraint
    return false;

  if ( hinfo )
  {
    int sample_rate(dts_sample_rates[sfreq]);
    int mask(amode2mask_tbl[amode]);
    const int relation(amode2rel_tbl[amode]);

    if ( lff )
    {
      mask |= CH_MASK_LFE;
    }

#if 0
    if ( hdLen )
    {
      sample_rate *= 4;
      mask = MODE_7_1;
    }
#endif

    hinfo->setSpeakers(Speakers(FORMAT_DTS, mask, sample_rate, 1.0, relation));
    hinfo->setFrameSize(fSiz + hdLen);
    hinfo->setDtsHdSize(hdLen);
    hinfo->setScanSize(16384); // always scan up to maximum DTS frame size
    hinfo->setSampleCount(nblks * 32);
    hinfo->setBsType(bs_type);

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

