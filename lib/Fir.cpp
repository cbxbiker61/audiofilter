#include "Fir.h"

namespace {

const double zero(0.0);
const double one(1.0);

}; // anonymous namespace

namespace AudioFilter {

///////////////////////////////////////////////////////////////////////////////
// Constant generators

FIRZero fir_zero;
FIRIdentity fir_identity;

///////////////////////////////////////////////////////////////////////////////
// Special instance classes

ZeroFIRInstance::ZeroFIRInstance(int sample_rate_):
FIRInstance(sample_rate_, firt_zero, 1, 0, &zero)
{}

IdentityFIRInstance::IdentityFIRInstance(int sample_rate_):
FIRInstance(sample_rate_, firt_identity, 1, 0, &one)
{}

GainFIRInstance::GainFIRInstance(int sample_rate_, double gain_):
FIRInstance(sample_rate_, firt_gain, 1, 0, 0), gain(gain_)
{
  if ( gain == 0.0 )
    type = firt_zero;
  else if ( gain == 1.0 )
    type = firt_identity;

  data = &gain;
}

///////////////////////////////////////////////////////////////////////////////
// Special generator classes

const FIRInstance *
FIRZero::make(int sample_rate_) const
{
  return new ZeroFIRInstance(sample_rate_);
}

const FIRInstance *
FIRIdentity::make(int sample_rate_) const
{
  return new IdentityFIRInstance(sample_rate_);
}

const FIRInstance *
FIRGain::make(int sample_rate_) const
{
  return new GainFIRInstance(sample_rate_, gain);
}

FIRGain::FIRGain(): ver(0), gain(1.0)
{}

FIRGain::FIRGain(double gain_): ver(0), gain(gain_)
{}

void
FIRGain::setGain(double gain_)
{
  if ( gain != gain_ )
  {
    gain = gain_;
    ++ver;
  }
}

double
FIRGain::getGain(void) const
{
  return gain;
}

}; // namespace AudioFilter

// vim: ts=2 sts=2 et

