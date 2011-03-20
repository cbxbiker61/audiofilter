#include <AudioFilter/Speakers.h>

///////////////////////////////////////////////////////////////////////////////
// Just for reference
///////////////////////////////////////////////////////////////////////////////
/*
// special-purpose formats
#define FORMAT_UNKNOWN     (-1)
#define FORMAT_RAWDATA     0
#define FORMAT_LINEAR      1

// PCM low-endian formats
#define FORMAT_PCM16       2
#define FORMAT_PCM24       3
#define FORMAT_PCM32       4

// PCM big-endian formats
#define FORMAT_PCM16_BE    5
#define FORMAT_PCM24_BE    6
#define FORMAT_PCM32_BE    7

// PCM floating-point
#define FORMAT_PCMFLOAT    8
#define FORMAT_PCMDOUBLE   9

// container formats
#define FORMAT_PES        10 // MPEG1/2 Program Elementary Stream
#define FORMAT_SPDIF      11 // IEC 61937 stream

// compressed spdifable formats
#define FORMAT_MPA        12
#define FORMAT_AC3        13
#define FORMAT_DTS        14

// DVD LPCM
// Note: the sample size for this formats is doubled because
// LPCM samples are packed into blocks of 2 samples.
#define FORMAT_LPCM20     15
#define FORMAT_LPCM24     16
*/

namespace AudioFilter {

///////////////////////////////////////////////////////////////////////////////
// Constants for common audio formats
///////////////////////////////////////////////////////////////////////////////

extern const Speakers spk_unknown = Speakers(FORMAT_UNKNOWN, 0, 0, 0, 0);
extern const Speakers spk_rawdata = Speakers(FORMAT_RAWDATA, 0, 0, 0, 0);

///////////////////////////////////////////////////////////////////////////////
// Constants for common channel orders
///////////////////////////////////////////////////////////////////////////////

extern const int std_order[NCHANNELS] = { 0, 1, 2, 3, 4, 5 };

extern const int win_order[NCHANNELS] = { CH_L, CH_R, CH_C, CH_LFE, CH_SL, CH_SR };


///////////////////////////////////////////////////////////////////////////////
// Tables for Speakers class
///////////////////////////////////////////////////////////////////////////////

extern const int sample_size_tbl[32] =
{
  0,
  sizeof(sample_t),

  sizeof(int16_t),
  sizeof(int24_t),
  sizeof(int32_t),

  sizeof(int16_t),
  sizeof(int24_t),
  sizeof(int32_t),

  sizeof(float),
  sizeof(double),

  1, 1,             // PES/SPDIF
  1, 1, 1,          // MPA, AC3, DTS
  5, 6,             // DVD LPCM 20/24 bit

  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

extern const int mask_nch_tbl[64] =
{
  0, 1, 1, 2, 1, 2, 2, 3,
  1, 2, 2, 3, 2, 3, 3, 4,
  1, 2, 2, 3, 2, 3, 3, 4,
  2, 3, 3, 4, 3, 4, 4, 5,
  1, 2, 2, 3, 2, 3, 3, 4,
  2, 3, 3, 4, 3, 4, 4, 5,
  2, 3, 3, 4, 3, 4, 4, 5,
  3, 4, 4, 5, 4, 5, 5, 6
};

extern const int mask_order_tbl[64][6] =
{
{ CH_NONE },
{ CH_L },
{ CH_C },
{ CH_L, CH_C },
{ CH_R },
{ CH_L, CH_R },
{ CH_C, CH_R },
{ CH_L, CH_C, CH_R },
{ CH_SL },
{ CH_L, CH_SL },
{ CH_C, CH_SL },
{ CH_L, CH_C, CH_SL },
{ CH_R, CH_SL },
{ CH_L, CH_R, CH_SL },
{ CH_C, CH_R, CH_SL },
{ CH_L, CH_C, CH_R, CH_SL },
{ CH_SR },
{ CH_L, CH_SR },
{ CH_C, CH_SR },
{ CH_L, CH_C, CH_SR },
{ CH_R, CH_SR },
{ CH_L, CH_R, CH_SR },
{ CH_C, CH_R, CH_SR },
{ CH_L, CH_C, CH_R, CH_SR },
{ CH_SL, CH_SR },
{ CH_L, CH_SL, CH_SR },
{ CH_C, CH_SL, CH_SR },
{ CH_L, CH_C, CH_SL, CH_SR },
{ CH_R, CH_SL, CH_SR },
{ CH_L, CH_R, CH_SL, CH_SR },
{ CH_C, CH_R, CH_SL, CH_SR },
{ CH_L, CH_C, CH_R, CH_SL, CH_SR },
{ CH_LFE },
{ CH_L, CH_LFE },
{ CH_C, CH_LFE },
{ CH_L, CH_C, CH_LFE },
{ CH_R, CH_LFE },
{ CH_L, CH_R, CH_LFE },
{ CH_C, CH_R, CH_LFE },
{ CH_L, CH_C, CH_R, CH_LFE },
{ CH_SL, CH_LFE },
{ CH_L, CH_SL, CH_LFE },
{ CH_C, CH_SL, CH_LFE },
{ CH_L, CH_C, CH_SL, CH_LFE },
{ CH_R, CH_SL, CH_LFE },
{ CH_L, CH_R, CH_SL, CH_LFE },
{ CH_C, CH_R, CH_SL, CH_LFE },
{ CH_L, CH_C, CH_R, CH_SL, CH_LFE },
{ CH_SR, CH_LFE },
{ CH_L, CH_SR, CH_LFE },
{ CH_C, CH_SR, CH_LFE },
{ CH_L, CH_C, CH_SR, CH_LFE },
{ CH_R, CH_SR, CH_LFE },
{ CH_L, CH_R, CH_SR, CH_LFE },
{ CH_C, CH_R, CH_SR, CH_LFE },
{ CH_L, CH_C, CH_R, CH_SR, CH_LFE },
{ CH_SL, CH_SR, CH_LFE },
{ CH_L, CH_SL, CH_SR, CH_LFE },
{ CH_C, CH_SL, CH_SR, CH_LFE },
{ CH_L, CH_C, CH_SL, CH_SR, CH_LFE },
{ CH_R, CH_SL, CH_SR, CH_LFE },
{ CH_L, CH_R, CH_SL, CH_SR, CH_LFE },
{ CH_C, CH_R, CH_SL, CH_SR, CH_LFE },
{ CH_L, CH_C, CH_R, CH_SL, CH_SR, CH_LFE },
};

//extern const char *mode_text[64] =
const char *mode_text[64] =
{
  "-",
  "{ L }",
  "1/0 (mono)",
  "{ L, C }",
  "{ R }",
  "2/0 (stereo)",
  "{ C, R }",
  "3/0",
  "{ SL }",
  "{ L, SL }",
  "{ C, SL }",
  "{ L, C, SL }",
  "{ R, SL }",
  "2/1 (surround)",
  "{ C, R, SL }",
  "3/1 (surround)",
  "{ SR }",
  "{ L, SR }",
  "{ C, SR }",
  "{ L, C, SR }",
  "{ R, SR }",
  "{ L, R, SR }",
  "{ C, R, SR }",
  "{ L, C, R, SR }",
  "{ SL, SR }",
  "{ L, SL, SR }",
  "{ C, SL, SR }",
  "{ L, C, SL, SR }",
  "{ R, SL, SR }",
  "2/2 (quadro)",
  "{ C, R, SL, SR }",
  "3/2 (5 channels)",
  "{ LFE }",
  "{ L, LFE }",
  "1/0.1",
  "{ L, C, LFE }",
  "{ R, LFE }",
  "2/0.1 (2.1)",
  "{ C, R, LFE }",
  "3/0.1",
  "{ SL, LFE }",
  "{ L, SL, LFE }",
  "{ C, SL, LFE }",
  "{ L, C, SL, LFE }",
  "{ R, SL, LFE }",
  "2/1.1",
  "{ C, R, SL, LFE }",
  "3/1.1",
  "{ SR, LFE }",
  "{ L, SR, LFE }",
  "{ C, SR, LFE }",
  "{ L, C, SR, LFE }",
  "{ R, SR, LFE }",
  "{ L, R, SR, LFE }",
  "{ C, R, SR, LFE }",
  "{ L, C, R, SR, LFE }",
  "{ SL, SR, LFE }",
  "{ L, SL, SR, LFE }",
  "{ C, SL, SR, LFE }",
  "{ L, C, SL, SR, LFE }",
  "{ R, SL, SR, LFE }",
  "2/2.1 (4.1)",
  "{ C, R, SL, SR, LFE }",
  "3/2.1 (5.1)"
};

///////////////////////////////////////////////////////////////////////////////
// samples_t
///////////////////////////////////////////////////////////////////////////////

void samples_t::reorderToStd(Speakers _spk, const int _order[NCHANNELS])
{
  int mask = _spk.mask;
  sample_t *tmp[NCHANNELS];

  int ch = 0;
  for ( int i = 0; i < NCHANNELS; ++i )
  {
    if ( mask & CH_MASK(_order[i]) )
      tmp[_order[i]] = samples[ch++];
  }

  ch = 0;
  for (int i = 0; i < NCHANNELS; ++i )
  {
    if ( mask & CH_MASK(i) )
      samples[ch++] = tmp[i];
  }
}

void samples_t::reorderFromStd(Speakers _spk, const int _order[NCHANNELS])
{
  int mask = _spk.mask;
  sample_t *tmp[NCHANNELS];

  int ch = 0;
  for ( int i = 0; i < NCHANNELS; ++i )
  {
    if ( mask & CH_MASK(i) )
      tmp[i] = samples[ch++];
  }

  ch = 0;
  for ( int i = 0; i < NCHANNELS; ++i )
  {
    if ( mask & CH_MASK(_order[i]) )
      samples[ch++] = tmp[_order[i]];
  }
}

void samples_t::reorder(Speakers _spk
  , const int _input_order[NCHANNELS]
  , const int _output_order[NCHANNELS])
{
  int mask = _spk.mask;
  sample_t *tmp[NCHANNELS];

  int ch = 0;
  for ( int i = 0; i < NCHANNELS; ++i )
  {
    if ( mask & CH_MASK(_input_order[i]) )
      tmp[_input_order[i]] = samples[ch++];
  }

  ch = 0;
  for ( int i = 0; i < NCHANNELS; ++i )
  {
    if ( mask & CH_MASK(_output_order[i]) )
      samples[ch++] = tmp[_output_order[i]];
  }
}

}; // namespace AudioFilter

