#ifndef VALIB_CONVOLVER_H
#define VALIB_CONVOLVER_H

#include <math.h>
#include <AudioFilter/Buffer.h>
#include <AudioFilter/LinearFilter.h>
#include <AudioFilter/SyncHelper.h>
#include "../Fir.h"
#include "../dsp/Fft.h"

namespace AudioFilter {

///////////////////////////////////////////////////////////////////////////////
// Convolver class
// Use impulse response to implement FIR filtering.
///////////////////////////////////////////////////////////////////////////////

class Convolver : public LinearFilter
{
public:
  Convolver(const FIRGen *gen_ = 0);
  ~Convolver();

  /////////////////////////////////////////////////////////
  // Handle FIR generator changes

  void setFir(const FIRGen *gen_)
  {
    gen.set(gen_);
    reinit(false);
  }

  const FIRGen *getFir(void) const
  {
    return gen.get();
  }

  void releaseFir(void)
  {
    gen.release();
	reinit(false);
  }

  /////////////////////////////////////////////////////////
  // Filter interface

  virtual bool init(Speakers spk, Speakers &out_spk);
  virtual void resetState(void);

  virtual bool processSamples(samples_t in, size_t in_size, samples_t &out, size_t &out_size, size_t &gone);
  virtual bool flush(samples_t &out, size_t &out_size);

  virtual bool needFlushing(void) const;

protected:
  int ver;
  FIRRef gen;
  const FIRInstance *fir;
  SyncHelper sync_helper;

  int buf_size;
  int n, c;
  int pos;

  FFT       fft;
  Samples   filter;
  SampleBuf buf;
  Samples   fft_buf;

  int pre_samples;
  int post_samples;

  bool firChanged(void) const;
  void uninit(void);
  void convolve(void);

  enum { state_filter, state_zero, state_pass, state_gain } state;

};

}; // namespace AudioFilter

#endif

