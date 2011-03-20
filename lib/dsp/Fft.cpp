#include <math.h>
#include "Fft.h"
#include "FftSg.h"

FFT::FFT()
{}

FFT::FFT(unsigned length)
{
  set_length(length);
}

bool FFT::set_length(unsigned length)
{
  if (len == length && is_ok())
    return true;

  len = length;
  fft_ip.allocate((int)(2 + sqrt(double(length * 2))));
  fft_w.allocate(length/2+1);

  if (fft_ip.is_allocated())
    fft_ip[0] = 0;

  return is_ok();
}

void FFT::rdft(sample_t *samples)
{
  ::rdft(len, 1, samples, fft_ip, fft_w);
}

void FFT::inv_rdft(sample_t *samples)
{
  ::rdft(len, -1, samples, fft_ip, fft_w);
}

