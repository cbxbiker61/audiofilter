#include <math.h>
#include <string.h>
#include "param_fir.h"
#include "../dsp/kaiser.h"

inline double sinc(double x) { return x == 0 ? 1 : sin(x)/x; }
inline double lpf(int i, double f) { return 2 * f * sinc(i * 2 * M_PI * f); }

ParamFIR::ParamFIR():
ver(0), type(0), f1(0.0), f2(0.0), df(0.0), a(0.0), norm(false)
{}

ParamFIR::ParamFIR(int _type, double _f1, double _f2, double _df, double _a, bool _norm):
ver(0), type(_type), f1(_f1), f2(_f2), df(_df), a(_a), norm(_norm)
{}

void
ParamFIR::set(int _type, double _f1, double _f2, double _df, double _a, bool _norm)
{
  ver++;
  type = _type;
  f1 = _f1;
  f2 = _f2;
  df = _df;
  a  = _a;
  norm = _norm;

  if (type == FIR_BAND_PASS || type == FIR_BAND_STOP)
    if (f1 > f2)
    {
      double temp = f1;
      f1 = f2; f2 = temp;
    }
}

void
ParamFIR::get(int *_type, double *_f1, double *_f2, double *_df, double *_a, bool *_norm)
{
  if (_type) *_type = type;
  if (_f1)   *_f1 = f1;
  if (_f2)   *_f2 = f2;
  if (_df)   *_df = df;
  if (_a)    *_a  = a;
  if (_norm) *_norm = norm;
}

int
ParamFIR::version() const
{ 
  return ver; 
}

const FIRInstance *
ParamFIR::make(int sample_rate) const
{
  int i;

  /////////////////////////////////////////////////////////////////////////////
  // Normalize

  double norm_factor = norm? 1.0: 1.0 / sample_rate;
  double f1_ = f1 * norm_factor;
  double f2_ = f2 * norm_factor;
  double df_ = df * norm_factor;

  /////////////////////////////////////////////////////////////////////////////
  // Trivial cases

  if (f1_ < 0.0 || f2_ < 0.0 || df_ <= 0.0 || a < 0.0) return 0;
  if (a == 0.0) return new IdentityFIRInstance(sample_rate);

  switch (type)
  {
    case FIR_LOW_PASS:
      if (f1_ >= 0.5) return new IdentityFIRInstance(sample_rate);
      if (f1_ == 0.0) return new GainFIRInstance(sample_rate, db2value(a));
      break;

    case FIR_HIGH_PASS:
      if (f1_ >= 0.5) return new GainFIRInstance(sample_rate, db2value(a));
      if (f1_ == 0.0) return new IdentityFIRInstance(sample_rate);
      break;

    case FIR_BAND_PASS:
      if (f1_ >= 0.5) return new GainFIRInstance(sample_rate, db2value(a));
      if (f2_ == 0.0) return new GainFIRInstance(sample_rate, db2value(a));
      if (f1_ == 0.0 && f2_ >= 0.5) return new IdentityFIRInstance(sample_rate);
      break;

    case FIR_BAND_STOP:
      if (f1_ >= 0.5) return new IdentityFIRInstance(sample_rate);
      if (f2_ == 0.0) return new IdentityFIRInstance(sample_rate);
      if (f1_ == 0.0 && f2_ >= 0.5) return new GainFIRInstance(sample_rate, db2value(a));
      break;

    default:
      return 0;
  }

  /////////////////////////////////////////////////////////////////////////////
  // Build the filter

  int n = kaiser_n(a, df_) | 1; // make odd (type 1 filter)
  int c = n / 2;

  double *filter = new double[n];
  if (!filter)
    return 0;

  memset(filter, 0, n * sizeof(double));
  filter[0] = 1.0;

  double alpha = kaiser_alpha(a);
  switch (type)
  {
    case FIR_LOW_PASS:
      for (i = 0; i < n; i++)
        filter[i] = (sample_t) (2 * f1_ * sinc((i - c) * 2 * M_PI * f1_) * kaiser_window(i - c, n, alpha));
      return new DynamicFIRInstance(sample_rate, firt_custom, n, c, filter);

    case FIR_HIGH_PASS:
      for (i = 0; i < n; i++)
        filter[i] = (sample_t) (-2 * f1_ * sinc((i - c) * 2 * M_PI * f1_) * kaiser_window(i - c, n, alpha));
      filter[c] = (sample_t) ((1 - 2 * f1_) * kaiser_window(0, n, alpha));
      return new DynamicFIRInstance(sample_rate, firt_custom, n, c, filter);

    case FIR_BAND_PASS:
      for (i = 0; i < n; i++)
        filter[i] = (sample_t) ((2 * f2_ * sinc((i - c) * 2 * M_PI * f2_) - 2 * f1_ * sinc((i - c) * 2 * M_PI * f1_)) * kaiser_window(i - c, n, alpha));
      return new DynamicFIRInstance(sample_rate, firt_custom, n, c, filter);

    case FIR_BAND_STOP:
      for (i = 0; i < n; i++)
        filter[i] = (sample_t) ((2 * f1_ * sinc((i - c) * 2 * M_PI * f1_) - 2 * f2_ * sinc((i - c) * 2 * M_PI * f2_)) * kaiser_window(i - c, n, alpha));
      filter[c] = (sample_t) ((2 * f1_ + 1 - 2 * f2_) * kaiser_window(0, n, alpha));
      return new DynamicFIRInstance(sample_rate, firt_custom, n, c, filter);
  };

  // never be here
  assert(false);
  delete filter;
  return 0;
}
