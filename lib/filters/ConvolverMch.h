#pragma once
#ifndef AUDIOFILTER_CONVOLVERMCH_H
#define AUDIOFILTER_CONVOLVERMCH_H

#include <math.h>
#include <AudioFilter/Buffer.h>
#include <AudioFilter/LinearFilter.h>
#include <AudioFilter/SyncHelper.h>
#include "../Fir.h"
#include "../dsp/Fft.h"

namespace AudioFilter {

///////////////////////////////////////////////////////////////////////////////
// Multichannel convolver class
// Use impulse response to implement FIR filtering.
///////////////////////////////////////////////////////////////////////////////

class ConvolverMch : public LinearFilter
{
public:
  ConvolverMch();
  ~ConvolverMch();

  /////////////////////////////////////////////////////////
  // Handle FIR generator changes

  void setFir(int ch_name, const FIRGen *gen);
  const FIRGen *getFir(int ch_name) const;
  void releaseFir(int ch_name);

  void setAllFirs(const FIRGen *gen[NCHANNELS]);
  void getAllFirs(const FIRGen *gen[NCHANNELS]);
  void releaseAllFirs(void);

  /////////////////////////////////////////////////////////
  // Filter interface

  virtual bool init(Speakers spk, Speakers &out_spk);
  virtual void resetState(void);

  virtual bool processSamples(samples_t in, size_t in_size, samples_t &out, size_t &out_size, size_t &gone);
  virtual bool flush(samples_t &out, size_t &out_size);

  virtual bool needFlushing(void) const;

protected:
  int ver[NCHANNELS];
  FIRRef gen[NCHANNELS];

  bool trivial;
  const FIRInstance *fir[NCHANNELS];
  enum { type_pass, type_gain, type_zero, type_conv } type[NCHANNELS];

  int buf_size;
  int n, c;
  int pos;

  FFT fft;
  SampleBuf filter;
  SampleBuf buf;
  Samples fft_buf;

  int pre_samples;
  int post_samples;

  bool firChanged(void) const;
  void uninit(void);

  void processTrivial(samples_t samples, size_t size);
  void processConvolve(void);

};

}; // namespace AudioFilter

#endif

