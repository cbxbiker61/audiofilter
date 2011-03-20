#include "source/generator.h"
#include "filters/slice.h"
#include "../../suite.h"

static const Speakers spk = Speakers(FORMAT_LINEAR, MODE_STEREO, 48000);
static const size_t noise_size = 64 * 1024;
static const int seed = 945098;

///////////////////////////////////////////////////////////////////////////////

TEST(slice, "SliceFilter test")

  int chunk_size = noise_size / 11;
  Chunk chunk;
  NoiseGen noise1;
  NoiseGen noise2;
  SliceFilter slice;

  /////////////////////////////////////////////////////////
  // Slice is equal to the stream

  noise1.init(spk, seed, noise_size, chunk_size);
  noise2.init(spk, seed, noise_size, chunk_size);
  slice.init(0, noise_size);

  CHECK(compare(log, &noise1, &slice, &noise2, 0) == 0);

  /////////////////////////////////////////////////////////
  // Slice is shorter than the stream

  noise1.init(spk, seed, noise_size, chunk_size);
  noise2.init(spk, seed, noise_size / 2, chunk_size);
  slice.init(0, noise_size / 2);

  CHECK(compare(log, &noise1, &slice, &noise2, 0) == 0);

  /////////////////////////////////////////////////////////
  // Slice is longer than the stream

  noise1.init(spk, seed, noise_size, chunk_size);
  noise2.init(spk, seed, noise_size, chunk_size);
  slice.init(0, 2 * noise_size);

  CHECK(compare(log, &noise1, &slice, &noise2, 0) == 0);

  /////////////////////////////////////////////////////////
  // Cut the end of the stream

  noise1.init(spk, seed, noise_size, chunk_size);
  noise2.init(spk, seed, noise_size, chunk_size);
  noise2.get_chunk(&chunk);
  slice.init(chunk_size, noise_size);

  CHECK(compare(log, &noise1, &slice, &noise2, 0) == 0);

  /////////////////////////////////////////////////////////
  // Cut the middle of the stream

  noise1.init(spk, seed, noise_size, chunk_size);
  noise2.init(spk, seed, noise_size / 2, chunk_size);
  noise2.get_chunk(&chunk);
  slice.init(chunk_size, noise_size / 2);

  CHECK(compare(log, &noise1, &slice, &noise2, 0) == 0);

  /////////////////////////////////////////////////////////
  // Cut after the end of the stream 1

  noise1.init(spk, seed, noise_size, chunk_size);
  noise2.init(spk, seed, noise_size, chunk_size);
  noise2.get_chunk(&chunk);
  slice.init(chunk_size, 2 * noise_size);

  CHECK(compare(log, &noise1, &slice, &noise2, 0) == 0);

  /////////////////////////////////////////////////////////
  // Cut after the end of the stream 2

  noise1.init(spk, seed, noise_size, chunk_size);
  noise2.init(spk, seed, 0, chunk_size);
  slice.init(2 * noise_size, 4 * noise_size);

  CHECK(compare(log, &noise1, &slice, &noise2, 0) == 0);

  /////////////////////////////////////////////////////////
  // Cut nothing at the beginning

  noise1.init(spk, seed, noise_size, chunk_size);
  noise2.init(spk, seed, 0, chunk_size);
  slice.init(0, 0);

  CHECK(compare(log, &noise1, &slice, &noise2, 0) == 0);

  /////////////////////////////////////////////////////////
  // Cut nothing in the middle

  noise1.init(spk, seed, noise_size, chunk_size);
  noise2.init(spk, seed, 0, chunk_size);
  slice.init(noise_size / 2, noise_size / 2);

  CHECK(compare(log, &noise1, &slice, &noise2, 0) == 0);

  /////////////////////////////////////////////////////////
  // Cut nothing at the end

  noise1.init(spk, seed, noise_size, chunk_size);
  noise2.init(spk, seed, 0, chunk_size);
  slice.init(noise_size, noise_size);

  CHECK(compare(log, &noise1, &slice, &noise2, 0) == 0);

  /////////////////////////////////////////////////////////
  // Cut nothing after the end

  noise1.init(spk, seed, noise_size, chunk_size);
  noise2.init(spk, seed, 0, chunk_size);
  slice.init(2 * noise_size, 2 * noise_size);

  CHECK(compare(log, &noise1, &slice, &noise2, 0) == 0);

TEST_END(slice);
