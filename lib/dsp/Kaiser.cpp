#include <math.h>
#include "dbesi0.h"
#include "Kaiser.h"

// We should limit the maximum attenuation, because window may become invalid
// due to the limited precision. Generally 80-bit double allows attenuations
// up to ~7000 dB. But I see no reason for such high value.
// Therefore we limit it on 1000 dB

static const double max_a = 1000;
static const double max_alpha = 109.24126;

double kaiser_alpha(double a)
{
  if (a <= 21) return 0; // rectangle window case
  if (a <= 50) return 0.5842 * pow(a - 21, 0.4) + 0.07886 * (a - 21);
  if (a > max_a) return max_alpha; // limit max attenuation
  return 0.1102 * (a - 8.7);
}

double kaiser_a(int n, double df)
{
  if (double(n) * df <= 0.9) return 21; // rectangle window case
  double a = 14.36 * df * double(n) + 7.95;
  return a < max_a? a: max_a; // limit max attenuation
}

int kaiser_n(double a, double df)
{
  a = fabs(a);
  if (a <= 21) return int(0.9 / df); // rectangle window case
  if (a > max_a) a = max_a; // limit max attenuation
  return int((a - 7.95) / (14.36 * df) + 1);
}

double kaiser_window(double i, int n, double alpha)
{
  if (alpha == 0.0) return 1.0; // rectangle window case

  double n1 = n - 1;
  if (fabs(2*i) > n1) return 0; // extend the window with zeros
  return dbesi0(alpha * sqrt(1 - 4*i*i / (n1 * n1))) / dbesi0(alpha);
}

