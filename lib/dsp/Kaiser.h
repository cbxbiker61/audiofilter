#ifndef VALIB_KAISER_H
#define VALIB_KAISER_H

/******************************************************************************

Kaiser window functions

* kaiser_alpha(a)
  a - attenuation in dB
  Returns parameter of a kaiser window for a given stopband attenuation

* kaiser_a(n, df)
  n - window length
  df - transition band width (normalized) 
  Returns maximum possible attenuation for a given window length and bandwidth

* kaiser_n(a, df)
  a - attenuation in dB
  df - transition band width (normalized)
  Returns minimum number of bins for the window to fulfil the given spec.

* kaiser_window(i, n, alpha)
  i - bin number
  n - window length
  alpha - window parameter
  Returns i-th bin of the window

******************************************************************************/

double kaiser_alpha(double a);
double kaiser_a(int n, double df);
int    kaiser_n(double a, double df);
double kaiser_window(double i, int n, double alpha);

#endif
