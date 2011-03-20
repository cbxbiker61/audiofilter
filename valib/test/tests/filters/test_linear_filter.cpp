#include <math.h>
#include "source/generator.h"
#include "filters/linear_filter.h"
#include "../../suite.h"

const size_t data_size = 65536;
const size_t block_size = 4096;

inline bool equal_time(vtime_t time1, vtime_t time2, vtime_t max_diff = 1e-6)
{ return fabs(time1 - time2) < max_diff; }

///////////////////////////////////////////////////////////////////////////////
// LinearFilterInplace - inplace filter
// LinearFilterBuffered - buffered filter
// Checks:
// * the correct sequence of calls
// * the correct sequence of data received

class LinearFilterTester : public LinearFilter
{
protected:
  mutable Speakers queried_spk;
  bool need_reset_after_init;
  bool need_reset_after_flushing;
  bool need_reinit;
  bool err;
  size_t seq;

public:
  LinearFilterTester():
    queried_spk(spk_unknown),
    need_reset_after_init(false),
    need_reset_after_flushing(false),
    need_reinit(false),
    err(false)
  {}

  void do_reinit() { need_reinit = true; reinit(false); }
  void reset_seq() { seq = 0; }
  bool is_ok() const { return !err; }

  virtual bool query(Speakers spk) const
  {
    queried_spk = spk;
    return true;
  }

  virtual bool init(Speakers spk, Speakers &out_spk)
  {
    need_reinit = false;
    need_reset_after_init = true;
    if (queried_spk != spk)
      err = true;
    return true;
  }

  virtual void reset_state()
  {
    need_reset_after_init = false;
    need_reset_after_flushing = false;
  }

};

class LinearFilterInplace : public LinearFilterTester
{
public:
  LinearFilterInplace() {}

  virtual bool process_inplace(samples_t in, size_t in_size)
  {
    if (need_reset_after_init) err = true;
    if (need_reset_after_flushing) err = true;
    if (need_reinit) err = true;
    if (int(in[0][0]) != seq) err = true;

    seq += in_size;
    return true;
  }
};

class LinearFilterBuffered : public LinearFilterTester
{
protected:
  SampleBuf buf;
  size_t pos;

public:
  LinearFilterBuffered(): pos(0) {}

  virtual bool init(Speakers spk, Speakers &out_spk)
  {
    pos = 0;
    buf.allocate(spk.nch(), block_size * 2);
    return LinearFilterTester::init(spk, out_spk);
  }

  virtual void reset_state()
  {
    pos = 0;
    LinearFilterTester::reset_state();
  }

  virtual bool process_samples(samples_t in, size_t in_size, samples_t &out, size_t &out_size, size_t &gone)
  {
    if (need_reset_after_init) err = true;
    if (need_reset_after_flushing) err = true;
    if (need_reinit) err = true;
    if (size_t(in[0][0]) != seq) err = true;

    if (pos >= 2 * block_size)
    {
      for (int ch = 0; ch < get_in_spk().nch(); ch++)
        memmove(buf[ch], buf[ch] + block_size, block_size * sizeof(sample_t));
      pos = block_size;
    }

    size_t n = block_size * 2 - pos;
    if (in_size < n)
    {
      for (int ch = 0; ch < get_in_spk().nch(); ch++)
        memcpy(buf[ch] + pos, in[ch], in_size * sizeof(sample_t));

      gone = in_size;
      seq += in_size;
      pos += in_size;
    }
    else
    {
      for (int ch = 0; ch < get_in_spk().nch(); ch++)
        memcpy(buf[ch] + pos, in[ch], n * sizeof(sample_t));

      out = buf;
      out_size = block_size;
      gone = n;
      seq += n;
      pos += n;
    }
    return true;
  }

  virtual bool flush(samples_t &out, size_t &out_size)
  {
    if (!need_flushing()) err = true;
    need_reset_after_flushing = true;

    if (pos >= 2 * block_size)
    {
      for (int ch = 0; ch < get_in_spk().nch(); ch++)
        memmove(buf[ch], buf[ch] + block_size, block_size * sizeof(sample_t));
      pos = block_size;
    }

    out = buf;
    out_size = pos;
    pos = 0;
    return true;
  }

  virtual bool need_flushing() const
  { return pos > 0; }
};

///////////////////////////////////////////////////////////////////////////////

TestResult linear_filter_test(Log *log, LinearFilterTester *f)
{
  Chunk chunk;
  LineGen gen;
  bool result;

  const Speakers spk_mono(FORMAT_LINEAR, MODE_MONO, 48000);
  const Speakers spk_stereo(FORMAT_LINEAR, MODE_STEREO, 48000);

  /////////////////////////////////////////////////////////
  // Input format change
  // Query format before init

  const Speakers spk_err_format(FORMAT_RAWDATA, MODE_STEREO, 48000);
  const Speakers spk_err_mode(FORMAT_LINEAR, 0, 48000);
  const Speakers spk_err_rate(FORMAT_LINEAR, MODE_STEREO, 0);

  result = f->set_input(spk_mono);
  CHECK(result && f->is_ok());

  result = f->set_input(spk_stereo);
  CHECK(result && f->is_ok());

  result = f->set_input(spk_err_format);
  CHECK(!result && f->is_ok());

  result = f->set_input(spk_err_mode);
  CHECK(!result && f->is_ok());

  result = f->set_input(spk_err_rate);
  CHECK(!result && f->is_ok());

  /////////////////////////////////////////////////////////
  // Reset after init
  /////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////
  // Set input format to a different one
  // filter must call init() and reset_state()
  // process checks that reset_state() was actually called

  gen.init(spk_stereo, 0, 1, data_size, block_size);
  gen.get_chunk(&chunk);
  f->reset_seq();
  f->set_input(spk_mono);
  f->set_input(spk_stereo);
  f->process(&chunk);
  CHECK(f->is_ok());

  /////////////////////////////////////////////////////////
  // Chunk has a different format
  // filter must call init() and reset_state()

  gen.init(spk_stereo, 0, 1, data_size, block_size);
  gen.get_chunk(&chunk);
  f->reset_seq();
  f->set_input(spk_mono);
  f->process(&chunk);
  CHECK(f->is_ok());

  /////////////////////////////////////////////////////////
  // Reinit
  /////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////
  // Reinit from init state

  gen.init(spk_stereo, 0, 1, data_size, block_size);
  f->reset_seq();
  f->set_input(spk_stereo);

  f->do_reinit();
  CHECK(f->is_empty());

  gen.get_chunk(&chunk);
  f->process(&chunk);
  CHECK(f->is_ok());

  /////////////////////////////////////////////////////////
  // Reinit immediately from full state

  gen.init(spk_stereo, 0, 1, data_size, block_size);
  gen.get_chunk(&chunk);
  f->reset_seq();
  f->set_input(spk_stereo);
  f->process(&chunk);

  f->do_reinit();
  CHECK(!f->is_empty());
  while (!f->is_empty())
    f->get_chunk(&chunk);

  gen.get_chunk(&chunk);
  f->process(&chunk);
  CHECK(f->is_ok());

  /////////////////////////////////////////////////////////
  // Reinit immediately from empty state

  gen.init(spk_stereo, 0, 1, data_size, block_size);
  gen.get_chunk(&chunk);
  f->reset_seq();
  f->set_input(spk_stereo);
  f->process(&chunk);
  while (!f->is_empty())
    f->get_chunk(&chunk);

  f->do_reinit();
  while (!f->is_empty())
    f->get_chunk(&chunk);

  gen.get_chunk(&chunk);
  f->process(&chunk);
  CHECK(f->is_ok());

  /////////////////////////////////////////////////////////
  // Reinit after flushing from flushing state

  gen.init(spk_stereo, 0, 1, data_size, block_size);
  gen.get_chunk(&chunk);
  chunk.set_eos();
  f->reset_seq();
  f->set_input(spk_stereo);
  f->process(&chunk);

  f->do_reinit();
  CHECK(!f->is_empty());
  while (!f->is_empty())
    f->get_chunk(&chunk);
  CHECK(chunk.eos);

  gen.get_chunk(&chunk);
  f->process(&chunk);
  CHECK(f->is_ok());

  /////////////////////////////////////////////////////////
  // Flushing
  /////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////
  // Init state, full chunk

  gen.init(spk_stereo, 0, 1, data_size, block_size);
  gen.get_chunk(&chunk);
  chunk.set_eos();

  f->reset_seq();
  f->set_input(spk_stereo);
  f->process(&chunk);

  CHECK(!f->is_empty());
  while (!f->is_empty())
    f->get_chunk(&chunk);
  CHECK(chunk.eos);

  gen.get_chunk(&chunk);
  f->process(&chunk);
  CHECK(f->is_ok());

  /////////////////////////////////////////////////////////
  // Init state, empty chunk

  gen.init(spk_stereo, 0, 1, data_size, block_size);
  chunk.set_empty(spk_stereo);
  chunk.set_eos();

  f->reset_seq();
  f->set_input(spk_stereo);
  f->process(&chunk);

  CHECK(!f->is_empty());
  while (!f->is_empty())
    f->get_chunk(&chunk);
  CHECK(chunk.eos);

  gen.get_chunk(&chunk);
  f->process(&chunk);
  CHECK(f->is_ok());

  /////////////////////////////////////////////////////////
  // Empty state, full chunk

  gen.init(spk_stereo, 0, 1, data_size, block_size);
  gen.get_chunk(&chunk);
  f->reset_seq();
  f->set_input(spk_stereo);
  f->process(&chunk);
  while (!f->is_empty())
    f->get_chunk(&chunk);

  gen.get_chunk(&chunk);
  chunk.set_eos();
  f->process(&chunk);

  CHECK(!f->is_empty());
  while (!f->is_empty())
    f->get_chunk(&chunk);
  CHECK(chunk.eos);

  gen.get_chunk(&chunk);
  f->process(&chunk);
  CHECK(f->is_ok());

  /////////////////////////////////////////////////////////
  // Empty state, empty chunk

  gen.init(spk_stereo, 0, 1, data_size, block_size);
  gen.get_chunk(&chunk);
  f->reset_seq();
  f->set_input(spk_stereo);
  f->process(&chunk);
  while (!f->is_empty())
    f->get_chunk(&chunk);

  chunk.set_empty(spk_stereo);
  chunk.set_eos();
  f->process(&chunk);

  CHECK(!f->is_empty());
  while (!f->is_empty())
    f->get_chunk(&chunk);
  CHECK(chunk.eos);

  gen.get_chunk(&chunk);
  f->process(&chunk);
  CHECK(f->is_ok());

  /////////////////////////////////////////////////////////
  // Timing test
  // Reinit after the first processing cycle

  const size_t block_multipliers[] =
  { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 16 };
  const int nmultipliers = array_size(block_multipliers);
  bool was_reinit = false;

  f->set_input(spk_stereo);
  CHECK(f->is_ok());

  for (int i = 0; i < nmultipliers * 2; i++)
  {
    size_t seq = 0;
    f->reset_seq();

    size_t current_chunk = 0;
    size_t current_block_size = i < nmultipliers? 
      block_size / block_multipliers[i]:
      block_size * block_multipliers[i - nmultipliers];

    gen.init(spk_stereo, 0, 1, data_size, current_block_size);
    while (!gen.is_empty())
    {
      if (seq >= block_size && !was_reinit)
      {
        f->do_reinit();
        was_reinit = true;
      }

      while (f->is_empty())
      {
        gen.get_chunk(&chunk);
        chunk.sync = true;
        chunk.time = vtime_t(current_chunk * 100);
        current_chunk++;

        if (gen.is_empty())
          CHECK(chunk.eos);
        f->process(&chunk);
      }
      CHECK(f->is_ok());

      while (!f->is_empty())
      {
        f->get_chunk(&chunk);
        CHECK(f->is_ok());
        if (chunk.size)
          CHECK(size_t(chunk.samples[0][0]) == seq);

        if (chunk.sync)
        {
          vtime_t chunk_time1 =
            vtime_t(seq / current_block_size) * 100 + 
            vtime_t(seq % current_block_size) / spk_stereo.sample_rate;
          vtime_t chunk_time2 = chunk_time1 - 100 + 
            vtime_t(current_block_size) / spk_stereo.sample_rate;

          CHECK(equal_time(chunk.time, chunk_time1) || equal_time(chunk.time, chunk_time2));
        }
        seq += chunk.size;
      }
    }
  }
  return test_passed;
}

///////////////////////////////////////////////////////////////////////////////

TEST(linear_filter_inplace, "LinearFilter inplace test")
  LinearFilterInplace f;
  linear_filter_test(log, &f);
TEST_END(linear_filter_inplace);

TEST(linear_filter_buffered, "LinearFilter buffered test")
  LinearFilterBuffered f;
  linear_filter_test(log, &f);
TEST_END(linear_filter_buffered);

///////////////////////////////////////////////////////////////////////////////
// Test suite
///////////////////////////////////////////////////////////////////////////////

SUITE(linear_filter, "LinearFilter")
  TEST_FACTORY(linear_filter_inplace),
  TEST_FACTORY(linear_filter_buffered),
SUITE_END;
