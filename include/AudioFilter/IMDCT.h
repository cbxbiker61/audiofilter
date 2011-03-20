#pragma once
#ifndef AUDIOFILTER_IMDCT_H
#define AUDIOFILTER_IMDCT_H

#include <math.h>
#include "Defs.h"

namespace AudioFilter {

typedef struct complex_s {
  sample_t real;
  sample_t imag;
} complex_t;

class IMDCT
{
protected:
  // Root values for IFFT
  sample_t roots16[3];
  sample_t roots32[7];
  sample_t roots64[15];
  sample_t roots128[31];

  // Twiddle factors for IMDCT
  complex_t pre1[128];
  complex_t post1[64];
  complex_t pre2[64];
  complex_t post2[32];

  complex_t buf128[128];
  complex_t *buf64_1;
  complex_t *buf64_2;

  // IFFT functions
  inline void ifft_pass(complex_t *buf, sample_t *weight, int n);
  inline void ifft2(complex_t *buf);
  inline void ifft4(complex_t *buf);
  inline void ifft8(complex_t *buf);
  void ifft16 (complex_t *buf);
  void ifft32 (complex_t *buf);
  void ifft64 (complex_t *buf);
  void ifft128(complex_t *buf);

  complex_t buf[128];

public:
  IMDCT();

  void imdct_512(sample_t *data, sample_t *delay);
  void imdct_256(sample_t *data, sample_t *delay);
};

// the basic split-radix ifft butterfly
#define BUTTERFLY(a0,a1,a2,a3,wr,wi) do {	\
  tmp5 = a2.real * wr + a2.imag * wi;		\
  tmp6 = a2.imag * wr - a2.real * wi;		\
  tmp7 = a3.real * wr - a3.imag * wi;		\
  tmp8 = a3.imag * wr + a3.real * wi;		\
  tmp1 = tmp5 + tmp7;				\
  tmp2 = tmp6 + tmp8;				\
  tmp3 = tmp6 - tmp8;				\
  tmp4 = tmp7 - tmp5;				\
  a2.real = a0.real - tmp1;			\
  a2.imag = a0.imag - tmp2;			\
  a3.real = a1.real - tmp3;			\
  a3.imag = a1.imag - tmp4;			\
  a0.real += tmp1;				\
  a0.imag += tmp2;				\
  a1.real += tmp3;				\
  a1.imag += tmp4;				\
} while (0)

// split-radix ifft butterfly, specialized for wr=1 wi=0
#define BUTTERFLY_ZERO(a0,a1,a2,a3) do {	\
  tmp1 = a2.real + a3.real;			\
  tmp2 = a2.imag + a3.imag;			\
  tmp3 = a2.imag - a3.imag;			\
  tmp4 = a3.real - a2.real;			\
  a2.real = a0.real - tmp1;			\
  a2.imag = a0.imag - tmp2;			\
  a3.real = a1.real - tmp3;			\
  a3.imag = a1.imag - tmp4;			\
  a0.real += tmp1;				\
  a0.imag += tmp2;				\
  a1.real += tmp3;				\
  a1.imag += tmp4;				\
} while (0)

// split-radix ifft butterfly, specialized for wr=wi
#define BUTTERFLY_HALF(a0,a1,a2,a3,w) do {	\
  tmp5 = (a2.real + a2.imag) * w;		\
  tmp6 = (a2.imag - a2.real) * w;		\
  tmp7 = (a3.real - a3.imag) * w;		\
  tmp8 = (a3.imag + a3.real) * w;		\
  tmp1 = tmp5 + tmp7;				\
  tmp2 = tmp6 + tmp8;				\
  tmp3 = tmp6 - tmp8;				\
  tmp4 = tmp7 - tmp5;				\
  a2.real = a0.real - tmp1;			\
  a2.imag = a0.imag - tmp2;			\
  a3.real = a1.real - tmp3;			\
  a3.imag = a1.imag - tmp4;			\
  a0.real += tmp1;				\
  a0.imag += tmp2;				\
  a1.real += tmp3;				\
  a1.imag += tmp4;				\
} while (0)

inline void
IMDCT::ifft2(complex_t *buf)
{
  double r, i;

  r = buf[0].real;
  i = buf[0].imag;
  buf[0].real += buf[1].real;
  buf[0].imag += buf[1].imag;
  buf[1].real = r - buf[1].real;
  buf[1].imag = i - buf[1].imag;
}

inline void
IMDCT::ifft4(complex_t *buf)
{
  double tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;

  tmp1 = buf[0].real + buf[1].real;
  tmp2 = buf[3].real + buf[2].real;
  tmp3 = buf[0].imag + buf[1].imag;
  tmp4 = buf[2].imag + buf[3].imag;
  tmp5 = buf[0].real - buf[1].real;
  tmp6 = buf[0].imag - buf[1].imag;
  tmp7 = buf[2].imag - buf[3].imag;
  tmp8 = buf[3].real - buf[2].real;

  buf[0].real = tmp1 + tmp2;
  buf[0].imag = tmp3 + tmp4;
  buf[2].real = tmp1 - tmp2;
  buf[2].imag = tmp3 - tmp4;
  buf[1].real = tmp5 + tmp7;
  buf[1].imag = tmp6 + tmp8;
  buf[3].real = tmp5 - tmp7;
  buf[3].imag = tmp6 - tmp8;
}

inline void
IMDCT::ifft8(complex_t *buf)
{
  double tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;

  ifft4 (buf);
  ifft2 (buf + 4);
  ifft2 (buf + 6);
  BUTTERFLY_ZERO (buf[0], buf[2], buf[4], buf[6]);
  BUTTERFLY_HALF (buf[1], buf[3], buf[5], buf[7], roots16[1]);
}

inline void
IMDCT::ifft_pass(complex_t *buf, sample_t *weight, int n)
{
  complex_t *buf1;
  complex_t *buf2;
  complex_t *buf3;
  double tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
  int i;

  buf++;
  buf1 = buf + n;
  buf2 = buf + 2 * n;
  buf3 = buf + 3 * n;

  BUTTERFLY_ZERO (buf[-1], buf1[-1], buf2[-1], buf3[-1]);

  i = n - 1;

  do {
    BUTTERFLY (buf[0], buf1[0], buf2[0], buf3[0], weight[n], weight[2*i]);
    ++buf;
    ++buf1;
    ++buf2;
    ++buf3;
    ++weight;
  } while ( --i );
}

}; // namespace AudioFilter

#endif

// vim: ts=2 sts=2 et

