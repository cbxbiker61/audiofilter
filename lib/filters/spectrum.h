/*
  Spectrum analysys filter
*/

#ifndef VALIB_SPECTRUM_H
#define VALIB_SPECTRUM_H

#include "../filter.h"
#include "../buffer.h"
#include "../dsp/fft.h"

class Spectrum : public NullFilter
{
protected:
  unsigned  length;
  FFT       fft;

  SampleBuf data;
  Samples   spectrum;
  Samples   win;

  size_t pos;
  bool is_ok;

  bool init();
  bool on_process();
  void on_reset();

public:
  Spectrum();

  unsigned get_length() const;
  bool     set_length(unsigned length);
  void     get_spectrum(int ch, sample_t *data, double *bin2hz);
};

#endif
