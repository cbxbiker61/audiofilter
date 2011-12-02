#include <AudioFilter/Parsers.h>

/*
  Minimum and maximum sized for MPEG frames:

  Layer      min br max br min sr max sr slots  min fsize max fsize
  -----------------------------------------------------------------
  MPEG1
  Layer1      32000 448000  32000  48000     12        32       672
  Layer2      32000 384000  32000  48000    144        96      1728
  Layer3      32000 320000  32000  48000    144        96      1440

  MPEG2/LSF
  Layer1      32000 256000  16000  24000     12        64       768
  Layer2       8000 160000  16000  24000    144        48      1440
  Layer3       8000 160000  16000  24000    144        48      1440
*/

namespace {

union RawHeader
{
  RawHeader() {}

  RawHeader(uint32_t i)
  {
    raw = i;
  }

  uint32_t raw;

  struct
  {
    unsigned emphasis           : 2;
    unsigned original           : 1;
    unsigned copyright          : 1;
    unsigned mode_ext           : 2;
    unsigned mode               : 2;

    unsigned extension          : 1;
    unsigned padding            : 1;
    unsigned sampling_frequency : 2;
    unsigned bitrate_index      : 4;

    unsigned error_protection   : 1;
    unsigned layer              : 2;
    unsigned version            : 1;
    unsigned sync               : 12;
  };
};

const int bitrate_tbl[2][3][15] =
{
  { // MPEG1
    { 0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448 },
    { 0, 32, 48, 56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 384 },
    { 0, 32, 40, 48,  56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320 }
  },
  { // MPEG2 LSF
    { 0, 32, 48, 56,  64,  80,  96, 112, 128, 144, 160, 176, 192, 224, 256 },
    { 0,  8, 16, 24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160 },
    { 0,  8, 16, 24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160 }
  }
};

const int freq_tbl[2][3] =
{
  { 44100, 48000, 32000 },  // MPEG1
  { 22050, 24000, 16000 }   // MPEG2 LSF
};

const int slots_tbl[3] = { 12, 144, 144 };

}; // anonymous namespace

namespace AudioFilter {

bool MpaHeaderParser::parseHeader(const uint8_t *hdr, HeaderInfo *hinfo)
{
  RawHeader h;
  int bs_type;

  // MPA big and little endians have ambigous headers
  // so first we check big endian as most used and only
  // then try little endian

  // 8 bit or 16 bit big endian stream sync
  if ((hdr[0] == 0xff)         && // sync
     ((hdr[1] & 0xf0) == 0xf0) && // sync
     ((hdr[1] & 0x06) != 0x00) && // layer
     ((hdr[2] & 0xf0) != 0xf0) && // bitrate
     ((hdr[2] & 0x0c) != 0x0c))   // sample rate
  {
    uint32_t header = *(uint32_t *)hdr;
    h = swab_u32(header);
    bs_type = BITSTREAM_8;
  }
  else
  // 16 bit little endian stream sync
  if ( (hdr[1] == 0xff)         && // sync
     ((hdr[0] & 0xf0) == 0xf0) && // sync
     ((hdr[0] & 0x06) != 0x00) && // layer
     ((hdr[3] & 0xf0) != 0xf0) && // bitrate
     ((hdr[3] & 0x0c) != 0x0c) )   // sample rate
  {
    uint32_t header = *(uint32_t *)hdr;
    h = (header >> 16) | (header << 16);
    bs_type = BITSTREAM_16LE;
  }
  else
    return false;

  if ( ! hinfo )
    return true;

  // common information
  int ver = 1 - h.version;
  int layer = 3 - h.layer;
  int bitrate = bitrate_tbl[ver][layer][h.bitrate_index] * 1000;
  int sample_rate = freq_tbl[ver][h.sampling_frequency];

  hinfo->setSpeakers(Speakers(FORMAT_MPA, (h.mode == 3)? MODE_MONO: MODE_STEREO, sample_rate));

  if ( bitrate )
  {
    hinfo->setFrameSize(bitrate * slots_tbl[layer] / sample_rate + h.padding);
    hinfo->setScanSize(0); // do not scan
  }
  else
  {
    hinfo->setFrameSize(0);
    hinfo->setScanSize(1728); // scan up to max frame size
  }

  if ( layer == 0 ) // MPA_LAYER_I
  {
    hinfo->setFrameSize(hinfo->getFrameSize() * 4);
  }

  hinfo->setSampleCount( (layer == 0) ? 384 : 1152);
  hinfo->setBsType(bs_type);

  if ( ver )
  {
    // MPEG2 LSF
    if (layer == 0)
      hinfo->setSpdifType(0x0008); // Pc burst-info (data type = MPEG2 Layer I LSF)
    else
      hinfo->setSpdifType(0x0009); // Pc burst-info (data type = MPEG2 Layer II/III LSF)
  }
  else
  {
    // MPEG1
    if ( layer == 0 )
      hinfo->setSpdifType(0x0004); // Pc burst-info (data type = MPEG1 Layer I)
    else
      hinfo->setSpdifType(0x0005); // Pc burst-info (data type = MPEG1/2 Layer II/III)
  }

  return true;
}

bool MpaHeaderParser::compareHeaders(const uint8_t *hdr1, const uint8_t *hdr2)
{
  // Compare following fields:
  // * layer
  // * protection bit
  // * sample rate
  // * mode must indicate the same number of channles (mono or stereo)
  static const int nch[4] = { 2, 2, 2, 1 };

  // 8 bit or 16 bit big endian stream sync
  if ((hdr1[0] == 0xff)         && // sync
     ((hdr1[1] & 0xf0) == 0xf0) && // sync
     ((hdr1[1] & 0x06) != 0x00) && // layer
     ((hdr1[2] & 0xf0) != 0xf0) && // bitrate
     ((hdr1[2] & 0x0c) != 0x0c))   // sample rate
  {
    return
      hdr1[0] == hdr2[0] &&
      hdr1[1] == hdr2[1] &&
      (hdr1[2] & 0x0c) == (hdr2[2] & 0x0c) && // sample rate
      nch[hdr1[3] >> 6] == nch[hdr2[3] >> 6]; // number of channels
  }
  else
  // 16 bit little endian stream sync
  if ((hdr1[1] == 0xff)         && // sync
     ((hdr1[0] & 0xf0) == 0xf0) && // sync
     ((hdr1[0] & 0x06) != 0x00) && // layer
     ((hdr1[3] & 0xf0) != 0xf0) && // bitrate
     ((hdr1[3] & 0x0c) != 0x0c))   // sample rate
  {
    return
      hdr1[1] == hdr2[1] &&
      hdr1[0] == hdr2[0] &&
      (hdr1[3] & 0x0c) == (hdr2[3] & 0x0c) && // sample rate
      nch[hdr1[2] >> 6] == nch[hdr2[2] >> 6]; // number of channels
  }
  else
    return false;
}

}; // namespace AudioFilter

// vim: ts=2 sts=2 et

