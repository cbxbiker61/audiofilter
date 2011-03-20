#pragma once
#ifndef AUDIOFILTER_CONVERTFUNC_H
#define AUDIOFILTER_CONVERTFUNC_H
/*
  PCM <-> Linear format conversion.
  Convert any supported PCM format into the internal format and vise versa.

  convert_t(uint8_t rawdata, samples_t samples, size_t size)
    Conversion function definition
    rawdata - PCM data pointer
    samples - Linear data pointers
    size - number of cycles to perform. Generally, this means number of samples
      but LPCM functions convert 2 samples per cycle.

  find_pcm2linear(int pcm_format, int nch)
    Find PCM to Linear conversion function.
    Returns zero if no conversion found.

  find_linear2pcm(int pcm_format, int nch)
    Find Linear to PCM conversion function.
    Returns zero if no conversion found.

  Note, that it is no guarantee that you can convert in both directions!
  Currently (20/06/2009) Linear to LPCM conversion is not supported.
*/

#include <AudioFilter/Speakers.h>

namespace AudioFilter {

typedef void (*convert_t)(uint8_t *, samples_t, size_t);
convert_t find_pcm2linear(int pcm_format, int nch);
convert_t find_linear2pcm(int pcm_format, int nch);

}; // namespace AudioFilter

#endif

