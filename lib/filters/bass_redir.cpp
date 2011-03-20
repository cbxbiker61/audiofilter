#include <math.h>
#include "bass_redir.h"

///////////////////////////////////////////////////////////////////////////////
// IIR, HPF, LPF
///////////////////////////////////////////////////////////////////////////////

void
IIR::process(sample_t *_samples, size_t _nsamples)
{
  double x, x1, x2, y, y1, y2;

  x1 = this->x1;
  x2 = this->x2;
  y1 = this->y1;
  y2 = this->y2;

  while (_nsamples--)
  {
    x = *_samples;
    y = a*x + a1*x1 + a2*x2 - b1*y1 - b2*y2;
    *_samples++ = y;

    x2 = x1;
    x1 = x;
    y2 = y1;
    y1 = y;
  }

  this->x1 = x1;
  this->x2 = x2;
  this->y1 = y1;
  this->y2 = y2;
}


void
HPF::update()
{
  if ((sample_rate < 10) || (freq < 10))
  {
    // setup as passthrough on incorrect parameters
    a  = 1.0;
    a1 = 0;
    a2 = 0;
    b1 = 0;
    b2 = 0;
    return;
  }

  double omega = 2.0 * M_PI * freq / sample_rate;
  double s = sin(omega);
  double c = cos(omega);
  double alfa = s * sinh(log(2.0) / 2.0 * omega / s);

  a  = gain * (1.0 + c) / 2.0 / (1.0 + alfa);
  a1 = gain * -(1.0 + c) / (1.0 + alfa);
  a2 = gain * (1.0 + c) / 2.0 / (1.0 + alfa);
  b1 = -(2.0 * c) / (1.0 + alfa);
  b2 = (1.0 - alfa) / (1.0 + alfa);

}

void
LPF::update()
{
  if ((sample_rate < 10) || (freq < 10))
  {
    // setup as passthrough on incorrect parameters
    a  = 1.0;
    a1 = 0;
    a2 = 0;
    b1 = 0;
    b2 = 0;
    return;
  }

  double omega = 2.0 * M_PI * freq / sample_rate;
  double s = sin(omega);
  double c = cos(omega);
  double alfa = s * sinh(log(2.0) / 2.0 * omega / s);

  a  = gain * (1.0 - c) / 2.0 / (1.0 + alfa);
  a1 = gain * (1.0 - c) / (1.0 + alfa);
  a2 = gain * (1.0 - c) / 2.0 / (1.0 + alfa);
  b1 = -(2.0 * c) / (1.0 + alfa);
  b2 = (1.0 - alfa) / (1.0 + alfa);
}



///////////////////////////////////////////////////////////////////////////////
// BassRedir
///////////////////////////////////////////////////////////////////////////////

BassRedir::BassRedir()
:NullFilter(FORMAT_MASK_LINEAR)
{
  // use default lpf filter setup (passthrough)
  enabled = false;
}


void
BassRedir::on_reset()
{
  lpf.reset();
}

bool
BassRedir::on_set_input(Speakers _spk)
{
  lpf.sample_rate = _spk.sample_rate;
  lpf.update();
  lpf.reset();

  return true;
}

bool 
BassRedir::on_process()
{
  if (!enabled || !spk.lfe())
    return true;

  int ch;
  int nch = spk.nch();

  // mix all channels to LFE
  for (ch = 0; ch < nch - 1; ch++)
  {
    sample_t *c   = samples[ch];
    sample_t *lfe = samples[nch-1];
    
    size_t i = size >> 2;
    while (i--)
    {
      lfe[0] += c[0];
      lfe[1] += c[1];
      lfe[2] += c[2];
      lfe[3] += c[3];
      lfe += 4;
      c += 4;
    }

    i = size & 3;
    while (i--)
      *lfe++ += *c++;
  }

  // Filter LFE channel
  lpf.process(samples[nch-1], size);

  return true;
}
