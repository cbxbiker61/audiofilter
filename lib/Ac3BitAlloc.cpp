// Reference bit allocation
// as it described in standard

#include "ac3_bitalloc.h"

#define DELTA_BIT_REUSE    0
#define DELTA_BIT_NEW      1
#define DELTA_BIT_NONE     2
#define DELTA_BIT_RESERVED 3

inline int logadd(int a, int b);
inline int calc_lowcomp(int a, int b0, int b1, int bin);
inline int min(int a, int b) { return (a > b)? b: a; }
inline int max(int a, int b) { return (a > b)? a: b; }
inline int abs(int a)        { return (a > 0)? a: -a; }

extern const uint8_t latab[260];
extern const int hth[3][50];
extern const uint8_t masktab[253];
extern const uint8_t bndtab[51];
extern const uint8_t bndsz[50];
extern const uint8_t baptab[64];

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
  int snroffset)

{
  int bin, lastbin, i, j, k, begin, bndstrt, bndend, lowcomp;
  int psd[256];   // PSD
  int bndpsd[50]; // integrated PSD
  int excite[50]; // excitation
  int mask[50];   // masking value

  // Step 1: Exponent mapping into PSD

  for (bin = start; bin < end; bin++)
    psd[bin] = (3072 - (exp[bin] << 7));

  // Step 2: PSD integration

  j = start;
  k = masktab[start];

  do
  {
    lastbin = min(bndtab[k] + bndsz[k], end);
    bndpsd[k] = psd[j];
    j++;

    for (i = j; i < lastbin; i++)
    {
      bndpsd[k] = logadd(bndpsd[k], psd[j]);
      j++;
    }

    k++;
  }
  while (end > lastbin);

  // Step 3: Compute excition function

  bndstrt = masktab[start];
  bndend = masktab[end - 1] + 1;

  if (bndstrt == 0) // for fbw and lfe channels
  {
    // note: do not calc_lowcomp() for the last band of the lfe channel, (bin = 6)
    lowcomp = 0;
    lowcomp = calc_lowcomp(lowcomp, bndpsd[0], bndpsd[1], 0);
    excite[0] = bndpsd[0] - fgain - lowcomp;
    lowcomp = calc_lowcomp(lowcomp, bndpsd[1], bndpsd[2], 1);
    excite[1] = bndpsd[1] - fgain - lowcomp;
    begin = 7;

    for (bin = 2; bin < 7; bin++)
    {
      if ((bndend != 7) || (bin != 6)) // skip for last bin of lfe channel
        lowcomp = calc_lowcomp(lowcomp, bndpsd[bin], bndpsd[bin+1], bin);

      fastleak = bndpsd[bin] - fgain;
      slowleak = bndpsd[bin] - sgain;
      excite[bin] = fastleak - lowcomp;

      if ((bndend != 7) || (bin != 6)) // skip for last bin of lfe channel
      {
        if (bndpsd[bin] <= bndpsd[bin+1])
        {
          begin = bin + 1;
          break;
        }
      }
    }

    for (bin = begin; bin < min(bndend, 22); bin++)
    {
      if ((bndend != 7) || (bin != 6)) // skip for last bin of lfe channel
        lowcomp = calc_lowcomp(lowcomp, bndpsd[bin], bndpsd[bin+1], bin);

      fastleak -= fdecay;
      fastleak = max(fastleak, bndpsd[bin] - fgain);
      slowleak -= sdecay;
      slowleak = max(slowleak, bndpsd[bin] - sgain);
      excite[bin] = max(fastleak - lowcomp, slowleak);
    }

    begin = 22;
  }
  else // for coupling channel
    begin = bndstrt;


  for (bin = begin; bin < bndend; bin++)
  {
    fastleak -= fdecay;
    fastleak = max(fastleak, bndpsd[bin] - fgain);
    slowleak -= sdecay;
    slowleak = max(slowleak, bndpsd[bin] - sgain);
    excite[bin] = max(fastleak, slowleak); 
  }

  // Step 4: Compute masking curve

  for (bin = bndstrt; bin < bndend; bin++)
  {
    if (bndpsd[bin] < dbknee)
      excite[bin] += ((dbknee - bndpsd[bin]) >> 2);
    mask[bin] = max(excite[bin], hth[fscod][bin >> halfratecod]);
  }

  // Step 5: Apply delta bit allocation

  if (deltbae == DELTA_BIT_NEW || deltbae == DELTA_BIT_REUSE)
    for (i = 0; i < 50; i++)
      mask[i] += deltba[i];

  // Step 6: Compute bit allocation

  i = start;
  j = masktab[start];

  do
  {
    lastbin = min(bndtab[j] + bndsz[j], end);
    mask[j] -= snroffset;

    mask[j] -= floor;
    if (mask[j] < 0)
      mask[j] = 0;

    mask[j] &= 0x1fe0; // 0001 1111 1110 0000
    mask[j] += floor;

    for (k = i; k < lastbin; k++)
    {
      int address = (psd[i] - mask[j]) >> 5;
      address = min(63, max(0, address));
      bap[i] = baptab[address];
      i++;
    }
    j++;
  }
  while (end > lastbin);

}


inline int logadd(int a, int b)
{
  int c = a - b;
  int address = min((abs(c) >> 1), 255);
  if (c >= 0)
    return a + latab[address];
  else
    return b + latab[address];

}

inline int calc_lowcomp(int a, int b0, int b1, int bin)
{
  if (bin < 7)
  {
    if ((b0 + 256) == b1)
      return 384;            // 180h 1_1000_0000b
    else if (b0 > b1)
      return max(0, a - 64); // 40h  0_0100_0000b
  }
  else if (bin < 20)
  {
    if ((b0 + 256) == b1)
      return 320;            // 140h 1_0100_0000b
    else if (b0 > b1)
      return max(0, a - 64); // 40h  0_0100_0000b      
  }
  else
    return max(0, a - 128);  // 80h  0_1000_0000b

  return a;
}


const uint8_t latab[260] =
{
  0x0040, 0x003f, 0x003e, 0x003d, 0x003c, 0x003b, 0x003a, 0x0039, 0x0038, 0x0037,
  0x0036, 0x0035, 0x0034, 0x0034, 0x0033, 0x0032, 0x0031, 0x0030, 0x002f, 0x002f,
  0x002e, 0x002d, 0x002c, 0x002c, 0x002b, 0x002a, 0x0029, 0x0029, 0x0028, 0x0027,
  0x0026, 0x0026, 0x0025, 0x0024, 0x0024, 0x0023, 0x0023, 0x0022, 0x0021, 0x0021,
  0x0020, 0x0020, 0x001f, 0x001e, 0x001e, 0x001d, 0x001d, 0x001c, 0x001c, 0x001b,
  0x001b, 0x001a, 0x001a, 0x0019, 0x0019, 0x0018, 0x0018, 0x0017, 0x0017, 0x0016,
  0x0016, 0x0015, 0x0015, 0x0015, 0x0014, 0x0014, 0x0013, 0x0013, 0x0013, 0x0012,
  0x0012, 0x0012, 0x0011, 0x0011, 0x0011, 0x0010, 0x0010, 0x0010, 0x000f, 0x000f,
  0x000f, 0x000e, 0x000e, 0x000e, 0x000d, 0x000d, 0x000d, 0x000d, 0x000c, 0x000c,
  0x000c, 0x000c, 0x000b, 0x000b, 0x000b, 0x000b, 0x000a, 0x000a, 0x000a, 0x000a,
  0x000a, 0x0009, 0x0009, 0x0009, 0x0009, 0x0009, 0x0008, 0x0008, 0x0008, 0x0008,
  0x0008, 0x0008, 0x0007, 0x0007, 0x0007, 0x0007, 0x0007, 0x0007, 0x0006, 0x0006,
  0x0006, 0x0006, 0x0006, 0x0006, 0x0006, 0x0006, 0x0005, 0x0005, 0x0005, 0x0005,
  0x0005, 0x0005, 0x0005, 0x0005, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004,
  0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003,
  0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0002,
  0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002,
  0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0001, 0x0001,
  0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001,
  0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001,
  0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};

const int hth[3][50] =
{
  {                 
    0x04d0, 0x04d0, 0x0440, 0x0400, 0x03e0, 0x03c0, 0x03b0, 0x03b0,  
    0x03a0, 0x03a0, 0x03a0, 0x03a0, 0x03a0, 0x0390, 0x0390, 0x0390,  
    0x0380, 0x0380, 0x0370, 0x0370, 0x0360, 0x0360, 0x0350, 0x0350,  
    0x0340, 0x0340, 0x0330, 0x0320, 0x0310, 0x0300, 0x02f0, 0x02f0,
    0x02f0, 0x02f0, 0x0300, 0x0310, 0x0340, 0x0390, 0x03e0, 0x0420,
    0x0460, 0x0490, 0x04a0, 0x0460, 0x0440, 0x0440, 0x0520, 0x0800,
    0x0840, 0x0840 
  },  
  { 0x04f0, 0x04f0, 0x0460, 0x0410, 0x03e0, 0x03d0, 0x03c0, 0x03b0, 
    0x03b0, 0x03a0, 0x03a0, 0x03a0, 0x03a0, 0x03a0, 0x0390, 0x0390, 
    0x0390, 0x0380, 0x0380, 0x0380, 0x0370, 0x0370, 0x0360, 0x0360, 
    0x0350, 0x0350, 0x0340, 0x0340, 0x0320, 0x0310, 0x0300, 0x02f0, 
    0x02f0, 0x02f0, 0x02f0, 0x0300, 0x0320, 0x0350, 0x0390, 0x03e0, 
    0x0420, 0x0450, 0x04a0, 0x0490, 0x0460, 0x0440, 0x0480, 0x0630, 
    0x0840, 0x0840 
  },  
  { 0x0580, 0x0580, 0x04b0, 0x0450, 0x0420, 0x03f0, 0x03e0, 0x03d0, 
    0x03c0, 0x03b0, 0x03b0, 0x03b0, 0x03a0, 0x03a0, 0x03a0, 0x03a0, 
    0x03a0, 0x03a0, 0x03a0, 0x03a0, 0x0390, 0x0390, 0x0390, 0x0390, 
    0x0380, 0x0380, 0x0380, 0x0370, 0x0360, 0x0350, 0x0340, 0x0330, 
    0x0320, 0x0310, 0x0300, 0x02f0, 0x02f0, 0x02f0, 0x0300, 0x0310, 
    0x0330, 0x0350, 0x03c0, 0x0410, 0x0470, 0x04a0, 0x0460, 0x0440, 
    0x0450, 0x04e0 
  }
};

const uint8_t masktab[253] =
{
   0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 
  16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 28, 28, 29, 
  29, 29, 30, 30, 30, 31, 31, 31, 32, 32, 32, 33, 33, 33, 34, 34, 
  34, 35, 35, 35, 35, 35, 35, 36, 36, 36, 36, 36, 36, 37, 37, 37, 
  37, 37, 37, 38, 38, 38, 38, 38, 38, 39, 39, 39, 39, 39, 39, 40, 
  40, 40, 40, 40, 40, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 
  41, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 43, 43, 43, 
  43, 43, 43, 43, 43, 43, 43, 43, 43, 44, 44, 44, 44, 44, 44, 44, 
  44, 44, 44, 44, 44, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 
  45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 46, 46, 46, 
  46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 
  46, 46, 46, 46, 46, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 
  47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 48, 48, 48, 
  48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 
  48, 48, 48, 48, 48, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 
  49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49 
};

const uint8_t bndtab[51] =
{
   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  
  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  
  20,  21,  22,  23,  24,  25,  26,  27,  28,  31,  
  34,  37,  40,  43,  46,  49,  55,  61,  67,  73,  
  79,  85,  97, 109, 121, 133, 157, 181, 205, 229, 
  0
};

const uint8_t bndsz[50] =
{
   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  
   1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  
   1,  1,  1,  1,  1,  1,  1,  1,  3,  3,  
   3,  3,  3,  3,  3,  6,  6,  6,  6,  6,  
   6, 12, 12, 12, 12, 24, 24, 24, 24, 24 
};


const uint8_t baptab[64] =
{
   0,  1,  1,  1,  1,  1,  2,  2,  
   3,  3,  3,  4,  4,  5,  5,  6,
   6,  6,  6,  7,  7,  7,  7,  8,  
   8,  8,  8,  9,  9,  9,  9, 10, 
  10, 10, 10, 11, 11, 11, 11, 12, 
  12, 12, 12, 13, 13, 13, 13, 14, 
  14, 14, 14, 14, 14, 14, 14, 15, 
  15, 15, 15, 15, 15, 15, 15, 15
};
