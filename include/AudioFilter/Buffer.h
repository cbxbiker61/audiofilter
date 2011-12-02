#pragma once
#ifndef AUDIOFILTER_BUFFER_H
#define AUDIOFILTER_BUFFER_H

/*
  Buffer allocation
  * RawBuf   - raw data buffer
  * Samples   - array of samples
  * SampleBuf - multichannel sample buffer
*/

#include "AutoBuf.h"
#include "Speakers.h"

namespace AudioFilter {

typedef AutoBuf<uint8_t> UInt8Buf;
typedef AutoBuf<sample_t> Samples;

class SampleBuf
{
public:
  SampleBuf()
    : f_nch(0)
    , f_nsamples(0)
  {}

  SampleBuf(unsigned nch, size_t nsamples)
    : f_nch(0)
    , f_nsamples(0)
  {
    allocate(nch, nsamples);
  }

  inline bool allocate(unsigned nch, size_t nsamples)
  {
    if ( f_buf.allocate(nch * nsamples) == 0 )
    {
      free();
      return false;
    }

    f_nch = nch;
    f_nsamples = nsamples;
    f_samples.zero();

    for ( unsigned ch = 0; ch < nch; ++ch )
    {
      f_samples[ch] = f_buf.data() + ch * nsamples;
    }

    return true;
  }

  inline bool reallocate(unsigned nch, size_t nsamples)
  {
    unsigned min_nch = MIN(f_nch, nch);

    // Compact data before reallocation
    if ( min_nch > 1 && nsamples < f_nsamples )
    {
      for ( unsigned ch = 1; ch < min_nch; ++ch )
        memmove(f_buf + ch * nsamples, f_buf + ch * f_nsamples, nsamples * sizeof(sample_t));
    }

    // Reallocate
    if ( f_buf.reallocate(nch * nsamples) == 0 )
    {
      free();
      return false;
    }

    // Expand data after reallocation
    // Zero the tail
    if ( min_nch > 1 && nsamples > f_nsamples )
    {
      for ( unsigned ch = min_nch - 1; ch > 0; --ch )
      {
        memmove(f_buf + ch * nsamples, f_buf + ch * f_nsamples, f_nsamples * sizeof(sample_t));
        memset(f_buf + ch * nsamples + f_nsamples, 0, (nsamples - f_nsamples) * sizeof(sample_t));
      }
    }

    // Zero new channels
    if ( nch > f_nch )
      memset(f_buf.data() + f_nch * nsamples, 0, (nch - f_nch) * nsamples * sizeof(sample_t));

    // Update state
    f_nch = nch;
    f_nsamples = nsamples;

    for ( unsigned ch = 0; ch < nch; ++ch )
    {
      f_samples[ch] = f_buf.data() + ch * nsamples;
    }

    return true;
  }

  inline void free(void)
  {
    f_nch = 0;
    f_nsamples = 0;
    f_samples.zero();
    f_buf.free();
  }

  inline void zero(void)
  {
    f_buf.zero();
  }

  inline unsigned getChannelCount(void) const
  {
    return f_nch;
  }

  inline size_t getSampleCount(void) const
  {
    return f_nsamples;
  }

  inline samples_t getSamples(void) const
  {
    return f_samples;
  }

  inline bool isAllocated(void) const
  {
    return f_buf.isAllocated();
  }

  inline operator samples_t() const
  {
    return f_samples;
  }

  inline sample_t *operator [](unsigned ch) const
  {
    return f_samples[ch];
  }

protected:
  unsigned  f_nch;
  size_t    f_nsamples;
  samples_t f_samples;

  Samples f_buf;

};

} // namespace AudioFilter

#endif

// vim: ts=2 sts=2 et

