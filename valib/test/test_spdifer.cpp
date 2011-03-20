/*
  Spdifer filter test
  (using filter tester)

  1. Conversion test:
  1.1 high-bitrate DTS passthrough mode
  1.2 raw->spdif conversion
  1.3 spdif->spdif conversion

  2. Speed test on noise.

  3. Speed test on file:
  3.1 raw file
  3.2 spdif file

  Required files:
  test.mpa
  test.ac3
  test.dts
  test.mpa.spdif
  test.ac3.spdif
  test.dts.spdif
  test.all
  test.all.spdif

*/

#include "suite.h"
#include "filter_tester.h"
#include <filters/spdifer.h>
#include <source/generator.h>
#include <source/raw_source.h>
#include <win32/cpu.h>

///////////////////////////////////////////////////////////////////////////////
// Test constants

static const vtime_t time_per_test = 1.0; // 1 sec for each speed test

// noise speed test
static const int seed = 84735;
static const int noise_size = 10000000;

// file speed test
static const char *file_raw   = "a.mad.mix.mad";
static const char *file_spdif = "a.mad.mix.spdif";

///////////////////////////////////////////////////////////////////////////////
// Test class

class Spdifer_test
{
protected:
  FilterTester t;
  Spdifer spdifer;
  Log *log;

public:
  Spdifer_test(Log *_log)
  {
    log = _log;
    t.link(&spdifer, log);
  }

  int test()
  {
    log->open_group("Spdifer test");
    transform();
    speed_noise();
    speed_file(file_raw);
    speed_file(file_spdif);
    return log->close_group();
  }

  void transform()
  {
    /////////////////////////////////////////////////////////
    // Transform test

    // high-bitrate dts passthrough test
    compare_file(log, Speakers(FORMAT_DTS, 0, 0), "e:\\samples\\dts\\DTS-AudioCD\\Music\\Mendelssohn_2.dts", &t, "e:\\samples\\dts\\DTS-AudioCD\\Music\\Mendelssohn_2.dts");
    // raw stream -> spdif stream
    compare_file(log, Speakers(FORMAT_MPA, 0, 0), "a.mp2.005.mp2", &t, "a.mp2.005.spdif");
    compare_file(log, Speakers(FORMAT_MPA, 0, 0), "a.mp2.002.mp2", &t, "a.mp2.002.spdif");
    compare_file(log, Speakers(FORMAT_AC3, 0, 0), "a.ac3.03f.ac3", &t, "a.ac3.03f.spdif");
    compare_file(log, Speakers(FORMAT_AC3, 0, 0), "a.ac3.005.ac3", &t, "a.ac3.005.spdif");
    compare_file(log, Speakers(FORMAT_DTS, 0, 0), "a.dts.03f.dts", &t, "a.dts.03f.spdif");
    compare_file(log, Speakers(FORMAT_RAWDATA, 0, 0), "a.mad.mix.mad", &t, "a.mad.mix.spdif");
    // spdif stream -> spdif stream
    compare_file(log, Speakers(FORMAT_SPDIF, 0, 0), "a.mp2.005.spdif", &t, "a.mp2.005.spdif");
    compare_file(log, Speakers(FORMAT_SPDIF, 0, 0), "a.mp2.002.spdif", &t, "a.mp2.002.spdif");
    compare_file(log, Speakers(FORMAT_SPDIF, 0, 0), "a.ac3.03f.spdif", &t, "a.ac3.03f.spdif");
    compare_file(log, Speakers(FORMAT_SPDIF, 0, 0), "a.ac3.005.spdif", &t, "a.ac3.005.spdif");
    compare_file(log, Speakers(FORMAT_SPDIF, 0, 0), "a.dts.03f.spdif", &t, "a.dts.03f.spdif");
    compare_file(log, Speakers(FORMAT_SPDIF, 0, 0), "a.mad.mix.spdif", &t, "a.mad.mix.spdif");
  }

  void speed_noise()
  {
    /////////////////////////////////////////////////////////
    // Noise speed test

    Chunk ichunk;
    Chunk ochunk;
    NoiseGen noise(Speakers(FORMAT_AC3, 0, 0), seed, noise_size, noise_size);
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

    log->msg("Spdifer speed on noise: %iMB/s, Data: %i, Empty: %i", 
      int(double(noise_size) * runs / cpu.get_thread_time() / 1000000), 
      data_chunks / runs, empty_chunks / runs);
  }

  void speed_file(const char *file_name)
  {
    /////////////////////////////////////////////////////////
    // File speed test

    Chunk ichunk;
    Chunk ochunk;
    RAWSource f(Speakers(FORMAT_RAWDATA, 0, 0), file_name);
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

    log->msg("Spdifer speed on file %s: %iMB/s, Data: %i, Empty: %i", file_name, 
      int(double(f.size()) * runs / cpu.get_thread_time() / 1000000), 
      data_chunks / runs, empty_chunks / runs);
  }

};

///////////////////////////////////////////////////////////////////////////////
// Test function

int test_spdifer(Log *log)
{
  Spdifer_test test(log);
  return test.test();
}

