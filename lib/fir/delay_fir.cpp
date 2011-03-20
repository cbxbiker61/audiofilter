#include "delay_fir.h"

DelayFIR::DelayFIR()
:ver(0), delay(0)
{}

DelayFIR::DelayFIR(vtime_t delay_)
:ver(0), delay(0)
{
  set_delay(delay_);
}

void
DelayFIR::set_delay(vtime_t delay_)
{
  delay = delay_;
  if (delay < 0) delay = 0;
  ver++;
}

vtime_t
DelayFIR::get_delay() const
{
  return delay;
}

int DelayFIR::version() const
{
  return ver;
}

const FIRInstance *
DelayFIR::make(int sample_rate) const
{
  int samples = int(delay * sample_rate);

  if (samples == 0)
    return new IdentityFIRInstance(sample_rate);

  double *data = new double[samples+1];
  if (!data) return 0;

  data[samples] = 1.0;
  for (int i = 0; i < samples; i++)
    data[i] = 0;

  return new DynamicFIRInstance(sample_rate, firt_custom, samples+1, 0, data);
}
