#pragma once
#ifndef AUDIOFILTER_AC3BITALLOC_H
#define AUDIOFILTER_AC3BITALLOC_H

/*
  AC3 bit allocation
*/

#include <AudioFilter/Defs.h>

namespace AudioFilter {

void bit_alloc(
  int8_t *bap,    // [256]
  int8_t *exp,    // [256]
  int deltbae,
  int8_t *deltba, // [50]
  int start, int end,
  int fscod, int halfratecod,
  int sdecay, int fdecay,
  int sgain, int fgain,
  int dbknee, int floor,
  int fastleak, int slowleak,
  int snroffset);

class BAP_BitCount
{
protected:
  int cnt1, cnt2, cnt4;

public:
  int bits;

  BAP_BitCount(): cnt1(0), cnt2(0), cnt4(0), bits(0) {};

  inline void reset()
  {
    cnt1 = 0;
    cnt2 = 0;
    cnt4 = 0;
    bits = 0;
  }

  inline void add_bap(int8_t *bap, int start, int end)
  {
    while( start < end )
      add_bap(bap[start++]);
  }

  inline void add_bap(int8_t bap)
  {
    switch ( bap )
    {
      case 0:
        return;

      case 1:
        // 3-levels 3 values in 5 bits
        if ( ! cnt1-- )
        {
          bits += 5;
          cnt1 = 2;
        }
        return;

      case 2:
        // 5-levels 3 values in 7 bits
        if ( ! cnt2-- )
        {
          bits += 7;
          cnt2 = 2;
        }
        return;

      case 3:
        bits += 3;
        return;

      case 4:
        // 11-levels 2 values in 7 bits
        if ( ! cnt4-- )
        {
          cnt4 = 1;
          bits += 7;
        }
        return;

      case 5:
        bits += 4;
        return;
      case 14:
        bits += 14;
        return;
      case 15:
        bits += 16;
        return;

      default:
        bits += bap-1;
        return;
    }
  }
};

}; // namespace AudioFilter

#endif

