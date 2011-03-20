#pragma once
#ifndef AUDIOFILTER_GAINFILTER_H
#define AUDIOFILTER_GAINFILTER_H

/*
 * Simple gain filter
 */

#include "Filter.h"

namespace AudioFilter {

class GainFilter : public NullFilter
{
public:
  double gain;

  GainFilter(): NullFilter(FORMAT_MASK_LINEAR) {}
  GainFilter(double gain_): NullFilter(FORMAT_MASK_LINEAR), gain(gain_) {}

protected:
  virtual bool onProcess(void)
  {
    if ( gain != 1.0 )
    {
      for ( int ch = 0; ch < spk.nch(); ++ch )
        for ( size_t s = 0; s < size; ++s )
          samples[ch][s] *= gain;
    }

    return true;
  }
};

}; // namespace AudioFilter

#endif

// vim: ts=2 sts=2 et

