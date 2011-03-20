/*
  Base filter classes test
  (classes defined at filter.h)

  * Null filter must pass all data through
  * SourceFilter must act as a combination of a source and a filter
  * SinkFilter must act as a combination of a filter and a sink
*/

#include <math.h>
#include "source/generator.h"
#include "../suite.h"

static const int seed = 4796;
static const int noise_size = 64 * 1024;


// Filter zeroes all the data

class ZeroFilter : public NullFilter
{
public:
  ZeroFilter(): NullFilter(-1) {};

  bool get_chunk(Chunk *_chunk)
  {
    if (spk.is_linear())
    {
      for (int ch = 0; ch < spk.nch(); ch++)
        memset(samples[ch], 0, size * sizeof(sample_t));
    }
    else
      memset(rawdata, 0, size);

    send_chunk_inplace(_chunk, size);
    return true;
  }
};

// Sink accepts only zeroes and fails otherwise

class ZeroSink : public NullSink
{
public:
  ZeroSink() {};

  bool process(const Chunk *_chunk)
  {
    if (_chunk->is_dummy())
      return true;

    if (_chunk->spk.is_linear())
    {
      for (int ch = 0; ch < _chunk->spk.nch(); ch++)
        for (size_t i = 0; i < _chunk->size; i++)
          if (!EQUAL_SAMPLES(_chunk->samples[ch][i], 0.0))
            return false;
    }
    else
    {
      for (size_t i = 0; i < _chunk->size; i++)
        if (_chunk->rawdata[i] != 0)
          return false;
    }

    return true;
  }
};


///////////////////////////////////////////////////////////////////////////////
// NullFilter test
// Null filter must pass all data through
///////////////////////////////////////////////////////////////////////////////

TEST(base_null_filter, "NullFilter")
  Speakers spk;
  NoiseGen src_noise;
  NoiseGen ref_noise;
  NullFilter null_filter(-1);

  // Linear format test

  spk = Speakers(FORMAT_LINEAR, MODE_STEREO, 48000);
  src_noise.init(spk, seed, noise_size);
  ref_noise.init(spk, seed, noise_size);
  null_filter.reset();

  CHECK(compare(log, &src_noise, &null_filter, &ref_noise, 0) == 0);

  // Rawdata format test

  spk = Speakers(FORMAT_PCM16, MODE_STEREO, 48000);
  src_noise.init(spk, seed, noise_size);
  ref_noise.init(spk, seed, noise_size);
  null_filter.reset();

  CHECK(compare(log, &src_noise, &null_filter, &ref_noise, 0) == 0);

TEST_END(base_null_filter);

///////////////////////////////////////////////////////////////////////////////
// SourceFilter test
// SourceFilter must act as a combination of a source and a filter
///////////////////////////////////////////////////////////////////////////////

TEST(base_source_filter, "SourceFilter")

  Speakers spk(FORMAT_LINEAR, MODE_STEREO, 48000);
  
  NullFilter null_filter(-1);
  ZeroFilter zero_filter;

  NoiseGen src_noise;
  NoiseGen ref_noise;
  ZeroGen  ref_zero;

  // Init constructor test

  SourceFilter src_filter(&src_noise, &null_filter);
  CHECK(src_filter.get_source() == &src_noise);
  CHECK(src_filter.get_filter() == &null_filter);

  // Init test

  src_filter.set(&ref_zero, &zero_filter);
  CHECK(src_filter.get_source() == &ref_zero);
  CHECK(src_filter.get_filter() == &zero_filter);

  // Fail without a source

  CHECK(src_filter.set(0, 0) == false);

  // Noise source == Noise source (no filter)

  src_noise.init(spk, seed, noise_size);
  ref_noise.init(spk, seed, noise_size);

  CHECK(src_filter.set(&src_noise, 0) == true);
  CHECK(compare(log, &src_filter, &ref_noise) == 0);

  // Noise source + NullFilter == Noise source

  src_noise.init(spk, seed, noise_size);
  ref_noise.init(spk, seed, noise_size);
  null_filter.reset();

  CHECK(src_filter.set(&src_noise, &null_filter) == true);
  CHECK(compare(log, &src_filter, &ref_noise) == 0);

  // Noise source + ZeroFilter == Zero source

  src_noise.init(spk, seed, noise_size);
  ref_zero.init(spk, noise_size);
  zero_filter.reset();

  CHECK(src_filter.set(&src_noise, &zero_filter) == true);
  CHECK(compare(log, &src_filter, &ref_zero) == 0);
  
TEST_END(base_source_filter);

///////////////////////////////////////////////////////////////////////////////
// SinkFilter test
// SinkFilter must act as a combination of a filter and a sink
///////////////////////////////////////////////////////////////////////////////

TEST(base_sink_filter, "SinkFilter")
  Speakers spk(FORMAT_LINEAR, MODE_STEREO, 48000);
  
  NullFilter null_filter(-1);
  ZeroFilter zero_filter;
  NullSink   null_sink;
  ZeroSink   zero_sink;

  NoiseGen src_noise(spk, seed, noise_size);
  ZeroGen  src_zero(spk, noise_size);

  Chunk zero_chunk;
  Chunk noise_chunk;

  // Init constructor test

  SinkFilter sink_filter(&null_sink, &null_filter);
  CHECK(sink_filter.get_sink() == &null_sink);
  CHECK(sink_filter.get_filter() == &null_filter);

  // Init test

  sink_filter.set(&zero_sink, &zero_filter);
  CHECK(sink_filter.get_sink() == &zero_sink);
  CHECK(sink_filter.get_filter() == &zero_filter);

  // Fail without a sink

  CHECK(sink_filter.set(0, 0) == false);

  // Zero source == zero (no filter)
  // Noise source != zero (no filter)

  src_zero.get_chunk(&zero_chunk);
  src_noise.get_chunk(&noise_chunk);

  CHECK(sink_filter.set(&zero_sink, 0) == true);
  CHECK(sink_filter.process(&zero_chunk) == true);
  CHECK(sink_filter.process(&noise_chunk) == false);

  // Zero source + NullFilter == zero
  // Noise source + NullFilter != zero

  src_zero.get_chunk(&zero_chunk);
  src_noise.get_chunk(&noise_chunk);

  CHECK(sink_filter.set(&zero_sink, &null_filter) == true);
  CHECK(sink_filter.process(&zero_chunk) == true);
  CHECK(sink_filter.process(&noise_chunk) == false);

  // Zero source + ZeroFilter == zero
  // Noise source + ZeroFilter == zero

  src_zero.get_chunk(&zero_chunk);
  src_noise.get_chunk(&noise_chunk);

  CHECK(sink_filter.set(&zero_sink, &zero_filter) == true);
  CHECK(sink_filter.process(&zero_chunk) == true);
  CHECK(sink_filter.process(&noise_chunk) == true);

TEST_END(base_sink_filter);

///////////////////////////////////////////////////////////////////////////////
// Test suite
///////////////////////////////////////////////////////////////////////////////

SUITE(base, "Base filter classes test")
  TEST_FACTORY(base_null_filter),
  TEST_FACTORY(base_source_filter),
  TEST_FACTORY(base_sink_filter),
SUITE_END;
