#ifndef VALIB_AC3_MDCT_H
#define VALIB_AC3_MDCT_H

#include "../../defs.h"

class MDCT
{
protected:
  int16_t costab[64];
  int16_t sintab[64];
  int16_t fft_rev[512];
  int16_t xcos1[128];
  int16_t xsin1[128];

  struct IComplex 
  {
    short re,im;
  };

  void fft(IComplex *z, int ln);

public:
  MDCT(int ln);

  void mdct512(int32_t *out, int16_t *in);
};

#endif
