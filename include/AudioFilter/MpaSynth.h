#pragma once
#ifndef AUDIOFILTER_MPASYNTH_H
#define AUDIOFILTER_MPASYNTH_H
/*
 * Synthesis filter classes
 *
 * Implements synthesis filter for
 * MPEG1 Audio LayerI and LayerII
 */

#include "Defs.h"

namespace AudioFilter {

class SynthBuffer;
class SynthBufferFPU;
//class SynthBufferMMX;
//class SynthBufferSSE;
//class SynthBufferSSE2;
//class SynthBuffer3DNow;
//class SynthBuffer3DNow2;


///////////////////////////////////////////////////////////
// Synthesis filter interface

class SynthBuffer
{
public:
  virtual void synth(sample_t samples[32]) = 0;
  virtual void reset(void) = 0;
};


///////////////////////////////////////////////////////////
// FPU synthesis filter

class SynthBufferFPU: public SynthBuffer
{
public:
  SynthBufferFPU();

  virtual void synth(sample_t samples[32]);
  virtual void reset(void);

protected:
  sample_t synth_buf[1024]; // Synthesis buffer
  int synth_offset; // Offset in synthesis buffer

};

}; // namespace AudioFilter

#endif

// vim: ts=2 sts=2 et

