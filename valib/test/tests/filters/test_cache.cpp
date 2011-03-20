#include "../../suite.h"
#include "rng.h"
#include "filters/cache.h"
#include "source/generator.h"

static const Speakers spk = Speakers(FORMAT_LINEAR, MODE_STEREO, 48000);
const size_t data_size = 65536;
const size_t block_size = 4096;
const int seed = 349087593;

TEST(cache, "Cache filter")
  RNG rng(seed);
  Chunk chunk;
  LineGen gen;
  CacheFilter f;
  Samples buf;

  double block_multipliers[] = { 0.3, 0.5, 1, 2, 3 };
  int nmultipliers = array_size(block_multipliers);
  for (int i = 0; i < nmultipliers; i++)
    for (int j = 0; j < nmultipliers; j++)
    {
      size_t cache_samples = size_t(block_size * block_multipliers[j]);
      vtime_t cache_time = vtime_t(cache_samples) / spk.sample_rate;
      size_t chunk_size = size_t(block_size * block_multipliers[i]);

      gen.init(spk, 0, 1, data_size, chunk_size);
      buf.allocate(cache_samples);
      f.set_size(cache_time);
      while (!gen.is_empty())
      {
        gen.get_chunk(&chunk);
        f.process(&chunk);
        while (!f.is_empty())
          f.get_chunk(&chunk);

        size_t size = rng.get_range((uint32_t)cache_samples);
        vtime_t time = f.get_time() - vtime_t(size + rng.get_range((uint32_t)(cache_samples - size))) / spk.sample_rate;
        int time_samples = int(time * spk.sample_rate + 0.5);

        if (time < 0)
          // cache have no enough data
          continue;

        size = f.get_samples(CH_L, time, buf, size);
        if (size > 0)
        {
          CHECKT(int(buf[0] - time_samples) == 0, ("chunk_size = %i, cache_samples = %i, get time (samples) = %i", chunk_size, cache_samples, time_samples));
          CHECKT(int(buf[size-1] - (time_samples + size - 1)) == 0, ("i = %i, j = %i, get time (samples) = %i", chunk_size, cache_samples, time_samples));
        }
      }
    }
TEST_END(cache);
