/*
  Parametric filter impulse response:
  * Low-pass
  * High-pass
  * Band-pass
  * Band-stop
*/

#ifndef VALIB_PARAM_FIR_H
#define VALIB_PARAM_FIR_H

#include "../fir.h"

#define FIR_LOW_PASS  0
#define FIR_HIGH_PASS 1
#define FIR_BAND_PASS 2
#define FIR_BAND_STOP 3

class ParamFIR : public FIRGen
{
protected:
  int ver;   // response version
  int type;  // filter type
  double f1; // first bound frequency
  double f2; // second bound frequency (not used in low/high pass filters)
  double df; // transition band width
  double a;  // stopband attenuation (dB)
  bool norm; // normalized frequencies

public:
  ParamFIR();
  ParamFIR(int type, double f1, double f2, double df, double a, bool norm = false);

  void set(int  type, double  f1, double  f2, double  df, double  a, bool  norm = false);;
  void get(int *type, double *f1, double *f2, double *df, double *a, bool *norm = 0);;

  virtual int version() const;
  virtual const FIRInstance *make(int sample_rate) const;
};

#endif
