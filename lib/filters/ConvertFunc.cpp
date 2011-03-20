/*
  Notes about the conversion
  ==========================

  Integer PCM formats have asymmetrical range. For instance, PCM16 range is
  [-32768..32767]. Internal format assumes symmetrical range. This difference
  is important for AGC and clipping. If we use the range [-32767..+32767] we
  cannot maintain the roundtrip PCM16->Linear->PCM16 because AGC will gain if
  it see the sample with value -32768. If we use the range [-32768..+32768]
  we can pass the value +32768 and have a meet with overflow. Both cases are
  unacceptable. Therefore we use the shifted range [-32767.5..+32767.5].

  To convert between integer and floating-point we use following equations:
  s = i + 0.5
  i = round(s - 0.5) = floor(s)

  where round() means round to the nearest and floor() is round down (to -inf).

  This way may introduce a slight RC shift during the processing. For example
  gaining the signal produces the following shift:

  i = (i + 0.5) * gain - 0.5 = i * gain + 0.5 * (gain - 1) = gained_i + shift
  shift = 0.5 * (gain - 1)

  When gain < 1, the shift is insignificant. For gain >> 2 it may be noticable,
  but also do a useful job. Let's consider the conversion from PCM16 to PCM24.
  This process looks like: PCM16->Linear->Gain->PCM24. The conversion equation:

  pcm24 = (pcm16 + 0.5) * (8388607.5 / 32767.5) - 0.5

  This conversion fits the range of PCM16 into the range of PCM24 *exactly*.
  Standard conversion pcm24 = pcm16 * 256 leaves some space because
  32767 * 256 = 8388352 that is less than the maximum possible level 8388607.

  Similarly, conversion from PCM24 to PCM16:

  pcm16 = (pcm24 + 0.5) * (32767.5 / 8388607.5) - 0.5

  This conversion avoids the overflow possible with standard conversion:

  round(8388607 / 256) = round(32767.99) = 32768 (overflow; using floor()
  rounding in this case may solve the overflow problem, but introduce higer
  quantization noise)

  Rounding
  ========

  Default real to integer conversion is truncation. We cannot accept this
  because it will produce high quantization noise. Thus we have to specify
  the rounding directly. Direct call to floor() function is slow, but the only
  compatible solution. Also it is used in debug builds as reference. For
  release builds paltform-specific methods may be used. Following functions are
  wrappers for paltform-specific code:

  set_rounding() - set the correct rounding mode if needed
  release_rounding() - restore an old rounding mode if needed
  s2i() - sample to integer conversion
  i2s() - integer to sample conversion

  Note, that conversion DOES NOT do scaling. The correct level and no overflow
  guarantee is the task for the caller.
*/

#include <math.h>
#include "ConvertFunc.h"

namespace {

#if defined(_DEBUG) || !defined(_M_IX86)

inline int set_rounding(void)
{
  return 0;
}

inline void restore_rounding(int)
{
}

#define i2s(i) (sample_t(i)+0.5)
#define s2i(s) int32_t(floor(s))

#elif defined(_M_IX86)

inline int set_rounding(void)
{
  // Set x87 FPU rounding mode.
  // Returns unchanged FPU control word to restore later.

  const uint16_t rc_mask    = 0x0c00;
  const uint16_t rc_nearest = 0x0000;
  const uint16_t rc_down    = 0x0400;
  const uint16_t rc_up      = 0x0800;
  const uint16_t rc_trunc   = 0x0c00;

  uint16_t x87_ctrl;
  __asm fnstcw [x87_ctrl];

  uint16_t new_ctrl = x87_ctrl & ~rc_mask | rc_down;
  __asm fldcw [new_ctrl];

  return x87_ctrl;
}

inline void restore_rounding(int r)
{
  // Restore an old x87 FPU control word

  uint16_t x87_ctrl = r;
  __asm fldcw [x87_ctrl];
}

inline sample_t i2s(int32_t i)
{
  return sample_t(i) + 0.5;
}

inline int32_t s2i(sample_t s)
{
  register int32_t i;
  __asm fld [s]
  __asm fistp [i]
  return i;
}

#endif

}; // anonymous namespace

///////////////////////////////////////////////////////////////////////////////

#include "ConvertPcm2Linear.h"
#include "ConvertLinear2Pcm.h"

namespace AudioFilter {

///////////////////////////////////////////////////////////////////////////////

static const int pcm2linear_formats[] = { FORMAT_PCM16, FORMAT_PCM24, FORMAT_PCM32, FORMAT_PCM16_BE, FORMAT_PCM24_BE, FORMAT_PCM32_BE, FORMAT_PCMFLOAT, FORMAT_PCMDOUBLE, FORMAT_LPCM20, FORMAT_LPCM24 };
static const convert_t pcm2linear_tbl[NCHANNELS][10] = {
 { pcm16_linear_1ch, pcm24_linear_1ch, pcm32_linear_1ch, pcm16_be_linear_1ch, pcm24_be_linear_1ch, pcm32_be_linear_1ch, pcmfloat_linear_1ch, pcmdouble_linear_1ch, lpcm20_linear_1ch, lpcm24_linear_1ch },
 { pcm16_linear_2ch, pcm24_linear_2ch, pcm32_linear_2ch, pcm16_be_linear_2ch, pcm24_be_linear_2ch, pcm32_be_linear_2ch, pcmfloat_linear_2ch, pcmdouble_linear_2ch, lpcm20_linear_2ch, lpcm24_linear_2ch },
 { pcm16_linear_3ch, pcm24_linear_3ch, pcm32_linear_3ch, pcm16_be_linear_3ch, pcm24_be_linear_3ch, pcm32_be_linear_3ch, pcmfloat_linear_3ch, pcmdouble_linear_3ch, lpcm20_linear_3ch, lpcm24_linear_3ch },
 { pcm16_linear_4ch, pcm24_linear_4ch, pcm32_linear_4ch, pcm16_be_linear_4ch, pcm24_be_linear_4ch, pcm32_be_linear_4ch, pcmfloat_linear_4ch, pcmdouble_linear_4ch, lpcm20_linear_4ch, lpcm24_linear_4ch },
 { pcm16_linear_5ch, pcm24_linear_5ch, pcm32_linear_5ch, pcm16_be_linear_5ch, pcm24_be_linear_5ch, pcm32_be_linear_5ch, pcmfloat_linear_5ch, pcmdouble_linear_5ch, lpcm20_linear_5ch, lpcm24_linear_5ch },
 { pcm16_linear_6ch, pcm24_linear_6ch, pcm32_linear_6ch, pcm16_be_linear_6ch, pcm24_be_linear_6ch, pcm32_be_linear_6ch, pcmfloat_linear_6ch, pcmdouble_linear_6ch, lpcm20_linear_6ch, lpcm24_linear_6ch },
};

static const int linear2pcm_formats[] = { FORMAT_PCM16, FORMAT_PCM24, FORMAT_PCM32, FORMAT_PCM16_BE, FORMAT_PCM24_BE, FORMAT_PCM32_BE, FORMAT_PCMFLOAT, FORMAT_PCMDOUBLE };
static const convert_t linear2pcm_tbl[NCHANNELS][8] = {
 { linear_pcm16_1ch, linear_pcm24_1ch, linear_pcm32_1ch, linear_pcm16_be_1ch, linear_pcm24_be_1ch, linear_pcm32_be_1ch, linear_pcmfloat_1ch, linear_pcmdouble_1ch },
 { linear_pcm16_2ch, linear_pcm24_2ch, linear_pcm32_2ch, linear_pcm16_be_2ch, linear_pcm24_be_2ch, linear_pcm32_be_2ch, linear_pcmfloat_2ch, linear_pcmdouble_2ch },
 { linear_pcm16_3ch, linear_pcm24_3ch, linear_pcm32_3ch, linear_pcm16_be_3ch, linear_pcm24_be_3ch, linear_pcm32_be_3ch, linear_pcmfloat_3ch, linear_pcmdouble_3ch },
 { linear_pcm16_4ch, linear_pcm24_4ch, linear_pcm32_4ch, linear_pcm16_be_4ch, linear_pcm24_be_4ch, linear_pcm32_be_4ch, linear_pcmfloat_4ch, linear_pcmdouble_4ch },
 { linear_pcm16_5ch, linear_pcm24_5ch, linear_pcm32_5ch, linear_pcm16_be_5ch, linear_pcm24_be_5ch, linear_pcm32_be_5ch, linear_pcmfloat_5ch, linear_pcmdouble_5ch },
 { linear_pcm16_6ch, linear_pcm24_6ch, linear_pcm32_6ch, linear_pcm16_be_6ch, linear_pcm24_be_6ch, linear_pcm32_be_6ch, linear_pcmfloat_6ch, linear_pcmdouble_6ch },
};

convert_t find_pcm2linear(int pcm_format, int nch)
{
  if ( nch >= 1 && nch <= NCHANNELS )
  {
    for ( int i = 0; i < (int)array_size(pcm2linear_formats); ++i )
    {
      if ( pcm_format == pcm2linear_formats[i] )
        return pcm2linear_tbl[nch-1][i];
    }
  }

  return 0;
}

convert_t find_linear2pcm(int pcm_format, int nch)
{
  if ( nch >= 1 && nch <= NCHANNELS )
  {
    for ( int i = 0; i < (int)array_size(linear2pcm_formats); ++i )
    {
      if ( pcm_format == linear2pcm_formats[i] )
        return linear2pcm_tbl[nch-1][i];
    }
  }

  return 0;
}

}; // namespace AudioFilter

