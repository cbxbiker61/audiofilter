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

#if 0
/*
 * DTS type IV (DTS-HD) can be transmitted with various frame repetition
 * periods; longer repetition periods allow for longer packets and therefore
 * higher bitrate. Longer repetition periods mean that the constant bitrate of
 * the outputted IEC 61937 stream is higher.
 * The repetition period is measured in IEC 60958 frames (4 bytes).
 */
static int spdif_dts4_subtype(int period)
{
    switch (period) {
    case 512:   return 0x0;
    case 1024:  return 0x1;
    case 2048:  return 0x2;
    case 4096:  return 0x3;
    case 8192:  return 0x4;
    case 16384: return 0x5;
    }
    return -1;
}
#endif

}; // anonymous namespace

namespace AudioFilter {

bool DtsHdHeaderParser::parseHeader(const uint8_t *hdr, HeaderInfo *hi)
{
  // 16 bit big endian bitstream
  if ( hdr[0] == 0x64
        && hdr[1] == 0x58
        && hdr[2] == 0x20
        && hdr[3] == 0x25 )
  {
    if ( hi )
    {
      const int length(((hdr[6] & 0xf) << 11) + (hdr[7] << 3) + ((hdr[8] >> 5) & 7) + 1);
      const int sample_rate(48000); // not sure if the Hd Header has this or not
      hi->setBsType(BITSTREAM_16BE);
      hi->setSpeakers(Speakers(FORMAT_DTS, MODE_STEREO, sample_rate, 1.0, NO_RELATION));
      //hi->frame_size = 0;
      hi->setFrameSize(length);
      hi->setScanSize(16384); // always scan up to maximum DTS frame size
      //hi->nsamples = nblks * 32;
      hi->setSampleCount(0); // should be a way to calc this
      hi->setSpdifType(20);
    }

    return true;
  }

  return false;

}

bool DtsHdHeaderParser::compareHeaders(const uint8_t *hdr1, const uint8_t *hdr2)
{
  // 16 bit big endian bitstream
  if ( hdr1[0] == 0x64
        && hdr1[1] == 0x58
        && hdr1[2] == 0x20
        && hdr1[3] == 0x25 )
  {
    return
      hdr1[0] == hdr2[0] && hdr1[1] == hdr2[1]
      && hdr1[2] == hdr2[2] && hdr1[3] == hdr2[3];
  }

  return false;
}

}; // namespace AudioFilter

// vim: ts=2 sts=2 et

