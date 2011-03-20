/*
  Random numbers generator test
  * Full proof of generator correctness by direct comparison of the entire
    sequence with reference Park-Miller implementation
*/

#include "rng.h"
#include "vtime.h"
#include "../suite.h"

TEST(rng, "Random numbers generator test")
  int i;
  const int seed = 9873485;
  int z;

  // Seed constructor test
  RNG rng1(seed);
  RNG rng2(seed);
  CHECKT(rng1.next() == rng2.next(), ("RNG::RNG(seed) constructor fails"));

  // RNG::seed() test
  rng2.seed(seed);
  rng2.next(); // one got at the first test
  CHECKT(rng1.next() == rng2.next(), ("RNG::seed() fails"));

  // Assignment test
  rng1.seed(123);
  rng2.seed(456);
  CHECK(rng1.next() != rng2.next());

  rng2 = rng1;
  CHECKT(rng1.next() == rng2.next(), ("Assignment operator fails"));

  // Try to seed with prohibited values

  rng1.seed(0);
  z = rng1.next();
  CHECKT(rng1.next() != z, ("Zero seed generates degenerated sequence"));

  rng1.seed((1U << 31) - 1);
  z = rng1.next();
  CHECKT(rng1.next() != z, ("Seed = 2^31-1 generates degenerated sequence"));

  rng1.seed(1U << 31);
  z = rng1.next();
  CHECKT(rng1.next() != z, ("Seed = 2^31 generates degenerated sequence"));

  // Randomize test

  for (i = 0; i < 1000; i++)
  {
    rng1.randomize();
    z = rng1.next();
    CHECKT(rng1.next() != z, ("Randomize causes degenerated sequence"));
  }

  // Quick Park&Miller test
  RNG rng(1);
  for (i = 0; i < 10000; i++)
    z = rng.next();
  CHECKT(z == 1043618065, ("Incorrect 'Minimal standard' implementation!"));

  // Raw fill test
  // Ensure no out-of-bound errors

  const int max_offset = 32;
  const int max_size = 256;
  const uint32_t guard = 0x55555555;
  uint32_t *guard1;
  uint32_t *guard2;

  const int buf_size = max_offset + max_size + sizeof(guard) * 2;
  uint8_t *buf = new uint8_t[buf_size];

  for (int offset = 0; offset < max_offset; offset++)
    for (int size = 0; size < max_size; size++)
    {
      guard1 = (uint32_t *)(buf + offset);
      uint8_t *data = buf + offset + sizeof(guard);
      guard2 = (uint32_t *)(buf + offset + size + sizeof(guard));

      *guard1 = guard;
      *guard2 = guard;

      rng.fill_raw(data, size);
  
      CHECKT(*guard1 == guard && *guard2 == guard, ("RNG::fill_raw() out of bound error"));
    }

TEST_END(rng);


inline uint32_t park_miller(uint32_t seed)
{
  static const int a = 16807;
  static const int m = 2147483647;
  static const int q = 127773; /* m div a */
  static const int r = 2836;   /* m mod a */

  int32_t hi = seed / q;
  int32_t lo = seed % q;
  int32_t test = a * lo - r * hi;
  return test > 0? test: test + m;
}

TEST(rng_proof, "Full proof of random numbers generator correctness")

  RNG rng;
  uint32_t seed;
  int i;

  /////////////////////////////////////////////////////////
  // Test Park-Miller implementation

  seed = 1;
  for (i = 0; i < 10000; i++)
    seed = park_miller(seed);

  CHECKT(seed == 1043618065, ("Park-Miller implementation is invalid"));

  /////////////////////////////////////////////////////////
  // Test our implementation

  rng.seed(1);
  for (i = 0; i < 10000-1; i++)
    rng.next();

  CHECKT(rng.next() == 1043618065, ("RNG implementation is invalid"));

  /////////////////////////////////////////////////////////
  // Compare entire sequence directly

  uint32_t count = 0;
  uint32_t max_count = 0x7fffffff;
  vtime_t start_time = local_time();
  while (count < max_count)
  {
    do {
      seed = park_miller(seed);
      CHECKT(seed == rng.next(), ("Test failed at %ith value", count));
    } while (++count & 0xfff);
    vtime_t dt = local_time() - start_time;
    vtime_t time_left = (max_count - count) * dt / count;
    log->status("Progress: %.1f%%\ttime left: %i:%i", float(count) * 100 / max_count, int(time_left) / 60, int(time_left) % 60);
  }

TEST_END(rng_proof)
