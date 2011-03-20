/*
  Demux filter test
  (using filter tester)

  1. Transform test
  1.1. pes to raw stream tests
  1.2. all possible pes format transitions test
  2. Speed test on noise
  3. Speed test on file
*/

#include "suite.h"
#include "filter_tester.h"
#include "filters\spdifer.h"
#include "filters\demux.h"
#include <source\generator.h>
#include <source\raw_source.h>
#include <win32\cpu.h>

///////////////////////////////////////////////////////////////////////////////
// Test constants

static const int seed = 98754;
static const int noise_size = 10000000;   // use 10MB noise buffer
static const vtime_t time_per_test = 1.0; // 1s per speed test
static const char *file_pes = "a.madp.mix.pes";

///////////////////////////////////////////////////////////////////////////////
// Test class

class Demux_test
{
protected:
  FilterTester t;
  Demux s;
  Log *log;

public:
  Demux_test(Log *_log)
  {
    log = _log;
    t.link(&s, log);
  }

  int test()
  {
    log->open_group("Demux test");
    transform();
    speed_noise();
    speed_file(file_pes);
    return log->close_group();
  }

  void transform()
  {
    /////////////////////////////////////////////////////////
    // Transform test

    compare_file(log, Speakers(FORMAT_PES, 0, 0), "a.mp2.005.pes", &t, "a.mp2.005.mp2");
    compare_file(log, Speakers(FORMAT_PES, 0, 0), "a.mp2.002.pes", &t, "a.mp2.002.mp2");
    compare_file(log, Speakers(FORMAT_PES, 0, 0), "a.mp2.mix.pes", &t, "a.mp2.mix.mp2");

    compare_file(log, Speakers(FORMAT_PES, 0, 0), "a.ac3.03f.pes", &t, "a.ac3.03f.ac3");
    compare_file(log, Speakers(FORMAT_PES, 0, 0), "a.ac3.005.pes", &t, "a.ac3.005.ac3");
    compare_file(log, Speakers(FORMAT_PES, 0, 0), "a.ac3.mix.pes", &t, "a.ac3.mix.ac3");

    compare_file(log, Speakers(FORMAT_PES, 0, 0), "a.dts.03f.pes", &t, "a.dts.03f.dts");

    compare_file(log, Speakers(FORMAT_PES, 0, 0), "a.pcm.005.pes", &t, "a.pcm.005.lpcm");

    compare_file(log, Speakers(FORMAT_PES, 0, 0), "a.mad.mix.pes", &t, "a.mad.mix.mad");
    compare_file(log, Speakers(FORMAT_PES, 0, 0), "a.madp.mix.pes", &t, "a.madp.mix.madp");
  }

  void speed_noise()
  {
    /////////////////////////////////////////////////////////
    // Noise speed test

    Chunk ichunk;
    Chunk ochunk;
    NoiseGen noise(Speakers(FORMAT_PES, 0, 0), seed, noise_size, noise_size);
    noise.get_chunk(&ichunk);

    CPUMeter cpu;
    cpu.reset();
    cpu.start();

    int runs = 0;
    int data_chunks = 0;
    int empty_chunks = 0;
    while (cpu.get_thread_time() < time_per_test)
    {
      runs++;
      t.reset();
      t.process(&ichunk);
      while (!t.is_empty())
      {
        t.get_chunk(&ochunk);
        if (ochunk.size)
          data_chunks++;
        else
          empty_chunks++;
      }
    }
    cpu.stop();

    log->msg("Demux speed on noise: %iMB/s, Data: %i, Empty: %i", 
      int(double(noise_size) * runs / cpu.get_thread_time() / 1000000), 
      data_chunks / runs, empty_chunks / runs);
  }

  void speed_file(const char *file_name)
  {
    /////////////////////////////////////////////////////////
    // File speed test

    Chunk ichunk;
    Chunk ochunk;
    RAWSource f(Speakers(FORMAT_PES, 0, 0), file_name);
    if (!f.is_open())
    {
      log->err("Cannot open file %s", file_name);
      return;
    }

    t.reset();

    CPUMeter cpu;
    cpu.reset();
    cpu.start();

    int runs = 0;
    int data_chunks = 0;
    int empty_chunks = 0;
    while (cpu.get_thread_time() < time_per_test)
    {
      runs++;
      f.seek(0);
      t.reset();
      while (!f.eof())
      {
        f.get_chunk(&ichunk);
        t.process(&ichunk);
        while (!t.is_empty())
        {
          t.get_chunk(&ochunk);
          if (ochunk.size)
            data_chunks++;
          else
            empty_chunks++;
        }
      }
    }
    cpu.stop();

    log->msg("Demux speed on file %s: %iMB/s, Data: %i, Empty: %i", file_name,
      int(double(f.size()) * runs / cpu.get_thread_time() / 1000000), 
      data_chunks / runs, empty_chunks / runs);
  }

};

///////////////////////////////////////////////////////////////////////////////
// Test function

int test_demux(Log *log)
{
  Demux_test test(log);
  return test.test();
}
