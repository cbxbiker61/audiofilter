#include "echo_fir.h"

EchoFIR::EchoFIR()
:ver(0), delay(0), gain(0)
{}

EchoFIR::EchoFIR(vtime_t delay_, double gain_)
:ver(0), delay(0), gain(0)
{
  set(delay_, gain_);
}

void
EchoFIR::set(vtime_t delay_, double gain_)
{
  delay = delay_;
  gain = gain_;
  if (delay < 0) delay = 0;
  ver++;
}

void
EchoFIR::get(vtime_t *delay_, double *gain_) const
{
  if (delay_) *delay_ = delay;
  if (gain_) *gain_ = gain;
}

void
EchoFIR::set_delay(vtime_t delay_)
{ set(delay_, gain); }

void
EchoFIR::set_gain(double gain_)
{ set(delay, gain_); }

vtime_t
EchoFIR::get_delay() const
{ return delay; }

double
EchoFIR::get_gain() const
{ return gain; }

int
EchoFIR::version() const
{
  return ver;
}

const FIRInstance *
EchoFIR::make(int sample_rate) const
{
  int samples = int(delay * sample_rate);

  if (samples == 0.0)
    if (gain == 1.0)
      return new IdentityFIRInstance(sample_rate);
    else
      return new GainFIRInstance(sample_rate, 1.0 + gain);

  double *data = new double[samples + 1];
  if (!data) return 0;

  data[0] = 1.0;
  data[samples] = gain;
  for (int i = 1; i < samples; i++)
    data[i] = 0;

  return new DynamicFIRInstance(sample_rate, firt_custom, samples+1, 0, data);
}
