#include <string.h>
#include "a52_parser.h"
#include "parsers\ac3\ac3_header.h"
#include "crc.h"

Speakers a52_to_spk(int _mode, int _sample_rate)
{
  const int acmod2mask_tbl[] = 
  {
    MODE_2_0,
    MODE_1_0, 
    MODE_2_0,
    MODE_3_0,
    MODE_2_1,
    MODE_3_1,
    MODE_2_2,
    MODE_3_2,
    MODE_1_0,
    MODE_1_0,
    MODE_2_0,
  };

  int mask = (_mode & A52_LFE)? CH_MASK_LFE: 0;
  int mode = _mode & A52_CHANNEL_MASK;
  int dolby = NO_RELATION;
  switch (mode)
  {
  case A52_DOLBY:
    mask |= MODE_STEREO;
    dolby = RELATION_DOLBY;
    break;

  default:
    mask |= acmod2mask_tbl[mode];
    break;
  }
  return Speakers(FORMAT_LINEAR, mask, _sample_rate, 1.0, dolby);
}


A52Parser::A52Parser()
{
  frames = 0;
  errors = 0;

  samples.allocate(6, 1536);
  a52_state = a52_init(0);
  reset();
}

A52Parser::~A52Parser()
{
  a52_free(a52_state);
}

const HeaderParser *
A52Parser::header_parser() const
{
  return &ac3_header;
}




void 
A52Parser::reset()
{
}

bool
A52Parser::parse_frame(uint8_t *frame, size_t size)
{
  int mode, sample_rate, bitrate;
  int frame_size = a52_syncinfo(frame, &mode, &sample_rate, &bitrate);

  ///////////////////////////////////////////////////////
  // CRC check

  int frame_size1 = ((frame_size >> 1) + (frame_size >> 3)) & ~1;
  if (!calc_crc(0, frame,  frame_size1))
  {
    errors++;
    return false;
  }

  ///////////////////////////////////////////////////////
  // Start decoding

  sample_t level = 1.0;
  if (a52_frame(a52_state, frame, &mode, &level, 0.0))
  {
    errors++;
    return false;
  }

  ///////////////////////////////////////////////////////
  // Decode blocks

  int ch, b;
  for (b = 0; b < 6; b++)
  {
    if (a52_block(a52_state))
      return false;

    int nch = spk.nch();

    if (spk.lfe())
    {
      memcpy(samples[nch - 1] + b * 256, a52_samples(a52_state), sizeof(sample_t) * 256);
      for (ch = 0; ch < nch - 1; ch++)
        memcpy(samples[ch] + b * 256, a52_samples(a52_state) + (ch + 1) * 256, sizeof(sample_t) * 256);
    }
    else
      for (ch = 0; ch < nch; ch++)
        memcpy(samples[ch] + b * 256, a52_samples(a52_state) + ch * 256, sizeof(sample_t) * 256);
  }

  ///////////////////////////////////////////////////////
  // Decode blocks

  frames++;
  spk = a52_to_spk(mode, sample_rate);
  return true;
}

size_t
A52Parser::stream_info(char *buf, size_t size) const 
{
  if (buf && size) buf[0] = 0;
  return 0;
}

size_t
A52Parser::frame_info(char *buf, size_t size) const 
{
  if (buf && size) buf[0] = 0;
  return 0;
}

