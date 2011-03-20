#pragma once
#ifndef AUDIOFILTER_FFT_H
#define AUDIOFILTER_FFT_H

/*
 * Simple wrapper class for Ooura FFT
 */

#include <AudioFilter/Defs.h>
#include <AudioFilter/AutoBuf.h>

namespace AudioFilter {

class FFT
{
public:
  FFT();
  FFT(unsigned length);

  bool setLength(unsigned length);

  unsigned getLength(void) const
  {
    return len;
  }

  bool isOk(void) const
  {
    return fft_ip.isAllocated() && fft_w.isAllocated();
  }

  void rdft(sample_t *samples);
  void invRdft(sample_t *samples);

protected:
  AutoBuf<int> fft_ip;
  AutoBuf<sample_t> fft_w;
  unsigned len;

};

}; // namespace AudioFilter

#endif

// vim: ts=2 sts=2 et

