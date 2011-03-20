#pragma once
#ifndef AUDIOFILTER_RNG_H
#define AUDIOFILTER_RNG_H

#include "Defs.h"

/*
  Random number generator is based on:

  "Random Number Generators: Good Ones Are Hard to Find",
  S.K. Park and K.W. Miller, Communications of the ACM 31:10 (Oct 1988),

  "Two Fast Implementations of the 'Minimal Standard' Random Number Generator",
  David G. Carta, Comm. ACM 33, 1 (Jan 1990), p. 87-88 linear congruential
  generator f(z) = 16807 z mod (2 ** 31 - 1)

  Implementation
  ==============
  RNG implements Park&Miller's 'Minimal standard' generator:

  f(z) = (a * z) mod m
  where
    a = 16807 (most suggested by Park&Miller)
    m = 2^31 - 1

  It is a full-period generating function with sequence length of 2^31-2.
  Note, that values 0 and 2^31-1 are not included and cannot be used to seed
  the generator.

  Method
  ======
  Carta proposes following method to calculate the next value of the sequence:

  z := a * z
  p := hi_31_bit(z) // only 16bits used, because 'a' is just 16bit wide
  q := low_31_bit(z)
  z := p + q
  if (z >= 2^31)
    z := z - 2^31 + 1

  To avoid 'if' statement we can just add the most significant bit to the
  resulting value, so last 'if' statement turns into:

  z = clear_MSB(z + MSB(z));

  Most common tasks
  =================
  * Fill an array with raw/sample noise
  * Dithering noise
  * Random integers in a given range

  Plans
  =====
  Switch to Mersenne twister?
*/

namespace AudioFilter {

class RNG
{
public:
  RNG();
  RNG(int seed);
  RNG(const RNG &rng);
  RNG &operator =(const RNG &rng);

  RNG &seed(int seed);
  RNG &randomize(void);

  uint32_t getNext(void)
  {
    static const uint32_t a = 16807;
    uint32_t hi = a * (z >> 15);
    uint32_t lo = a * (z & 0x7fff);
    z = (hi >> 16) + ((hi << 15) & 0x7fffffff) + lo;
    z = (z & 0x7fffffff) + (z >> 31);
    return z;
  }

  uint32_t getRange(uint32_t range)
  {
    uint32_t t = getNext() << 1;
    uint32_t hi1 = t >> 16;
    uint32_t lo1 = t & 0xffff;
    uint32_t hi2 = range >> 16;
    uint32_t lo2 = range & 0xffff;
    return hi1*hi2 + ((hi1*lo2) >> 16) + ((hi2*lo1) >> 16);
  }

  sample_t getSample(void)
  {
    static const sample_t inv = 2.0 / 2147483646.0; // 2^31 - 2 = 2147483646
    return (getNext() - 1) * inv - 1.0;
  }

  void fillRaw(void *data, size_t size);
  void fillSamples(sample_t *sample, size_t size);

protected:
  uint32_t z;

};

}; // namespace AudioFilter

#endif

// vim: ts=2 sts=2 et

