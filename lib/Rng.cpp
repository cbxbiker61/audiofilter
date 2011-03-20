#include <AudioFilter/VTime.h>
#include <AudioFilter/Rng.h>

namespace AudioFilter {

RNG::RNG()
{
  z = 1;
}

RNG::RNG(int _seed)
{
  seed(_seed);
}

RNG::RNG(const RNG &rng)
{
  z = rng.z;
}

RNG &RNG::operator =(const RNG &rng)
{
  z = rng.z;
  return *this;
}

///////////////////////////////////////////////////////////////////////////////
// Seeding

RNG &RNG::seed(int seed)
{
  z = seed;

  if ( z >= (1U << 31) - 1 ) // 2^31-1 and higher seeds are disallowed
    z = 1;

  if ( z == 0 ) // zero seed is disallowed
    z = 1;

  return *this;
}

RNG &RNG::randomize(void)
{
  // todo: use more than just a time to randomize?
  z = (uint32_t)(getLocalTime() * 1000) & 0x7fffffff;
  getNext();
  return *this;
}

///////////////////////////////////////////////////////////////////////////////
// Filling

void RNG::fillRaw(void *data, size_t size)
{
  size_t i;
  uint8_t  *ptr8;
  uint32_t *ptr32;
  size_t head_len, block_len, tail_len;

  // process small block
  if ( size < 32 )
  {
    ptr8 = (uint8_t *)data;

    for ( i = 0; i < size; ++i )
      ptr8[i] = getNext() >> 23; // use high 8 bits of 31-bit word

    return;
  }

  // arrange to 32-bit boundary
  ptr8 = (uint8_t *)data;
  head_len = align32(data);
  size -= head_len;

  for ( i = 0; i < head_len; ++i )
  {
    ptr8[i] = getNext() >> 23; // use high 8 bits of 31-bit word
  }

  // We must take in account that generator makes 31bit values. Therefore we
  // have to add one bit to each word. Generation of 2 values for each 32bit
  // word doubles the work. To avoid this we'll generate only 1 extra word for
  // each 8 words and unroll the main cycle.
  ptr32 = (uint32_t *)(ptr8 + head_len);
  block_len = (size >> 5) << 3; // in 32bit words, multiply of 8 words
  size -= block_len << 2;

  for ( i = 0; i < block_len; i += 8 )
  {
    uint32_t extra = getNext();
    ptr32[i+0] = getNext() | ((extra >> 30) << 31);
    ptr32[i+1] = getNext() | ((extra >> 29) << 31);
    ptr32[i+2] = getNext() | ((extra >> 28) << 31);
    ptr32[i+3] = getNext() | ((extra >> 27) << 31);
    ptr32[i+4] = getNext() | ((extra >> 26) << 31);
    ptr32[i+5] = getNext() | ((extra >> 25) << 31);
    ptr32[i+6] = getNext() | ((extra >> 24) << 31);
    ptr32[i+7] = getNext() | ((extra >> 23) << 31);
  }

  // Fill the tail
  ptr8 = (uint8_t *)(ptr32 + block_len);
  tail_len = size;

  for ( i = 0; i < tail_len; ++i )
  {
    ptr8[i] = getNext() >> 23; // use high 8 bits of 31-bit word
  }
}

void RNG::fillSamples(sample_t *sample, size_t size)
{
  for ( size_t i = 0; i < size; ++i )
    sample[i] = getSample();
}

}; // namespace AudioFilter

// vim: ts=2 sts=2 et

