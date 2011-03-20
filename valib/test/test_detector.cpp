/*
  Detector test
  * File passthrough test
  * Noise passthrough test
  * Speed test on file
  * Speed test on noise
  * todo: State transition test (code coverage)
*/

#include "suite.h"
#include "filter_tester.h"
#include "filters\detector.h"
#include <source\generator.h>
#include <source\raw_source.h>
#include <win32\cpu.h>

///////////////////////////////////////////////////////////////////////////////
// Test constants

static const int seed = 2347;
static const int noise_size = 1000000;     // noise buffer size
static const vtime_t time_per_test = 1.0;  // 1 sec for each speed test


///////////////////////////////////////////////////////////////////////////////
// Test class

class Detector_test
{
protected:
  Filter *f;
  FilterTester t;
  Detector detector;
  Log *log;

public:
  Detector_test(Log *_log)
  {
    log = _log;
    t.link(&detector, log);
    f = &detector; // do not use FilterTester
  }

  int test()
  {
    log->open_group("Detector test");
    passthrough_file();
    passthrough_noise();
    speed_noise();
    return log->close_group();
  }

  /////////////////////////////////////////////////////////////////////////////
  // Transform test

  void passthrough_file()
  {
    log->open_group("File passthrogh test");

    compare_file(log, Speakers(FORMAT_MPA, 0, 0), "a.mp2.mix.mp2", f, "a.mp2.mix.mp2");
    compare_file(log, Speakers(FORMAT_AC3, 0, 0), "a.ac3.mix.ac3", f, "a.ac3.mix.ac3");
    compare_file(log, Speakers(FORMAT_DTS, 0, 0), "a.dts.03f.dts", f, "a.dts.03f.dts");

    compare_file(log, Speakers(FORMAT_SPDIF, 0, 0), "a.mad.mix.spdif",  f, "a.mad.mix.spdif");
    compare_file(log, Speakers(FORMAT_SPDIF, 0, 0), "a.madp.mix.spdif", f, "a.madp.mix.spdif");

    compare_file(log, Speakers(FORMAT_PCM16, MODE_STEREO, 48000), "a.mad.mix.spdif",  f, "a.mad.mix.spdif");
    compare_file(log, Speakers(FORMAT_PCM16, MODE_STEREO, 48000), "a.madp.mix.spdif", f, "a.madp.mix.spdif");

    compare_file(log, Speakers(FORMAT_RAWDATA, 0, 0), "a.mad.mix.spdif",  f, "a.mad.mix.spdif");
    compare_file(log, Speakers(FORMAT_RAWDATA, 0, 0), "a.madp.mix.spdif", f, "a.madp.mix.spdif");
    compare_file(log, Speakers(FORMAT_RAWDATA, 0, 0), "a.mad.mix.mad",    f, "a.mad.mix.mad");
    compare_file(log, Speakers(FORMAT_RAWDATA, 0, 0), "a.madp.mix.madp",  f, "a.madp.mix.madp");

    log->close_group();
  }

  void passthrough_noise()
  {
    log->msg("Noise passthrogh test");

    Speakers spk = Speakers(FORMAT_RAWDATA, 0, 0);
    NoiseGen noise_src(spk, seed, noise_size);
    NoiseGen noise_ref(spk, seed, noise_size);

    f->set_input(spk);
    compare(log, &noise_src, f, &noise_ref);
  }

  void speed_noise()
  {
    /////////////////////////////////////////////////////////
    // Noise speed test

    Speakers spk = Speakers(FORMAT_PCM16, 0, 0);
    Chunk ichunk;
    Chunk ochunk;
    NoiseGen noise(spk, seed, noise_size, noise_size);
    noise.get_chunk(&ichunk);

    f->set_input(spk);

    CPUMeter cpu;
    cpu.reset();
    cpu.start();

    int runs = 0;
    int data_chunks = 0;
    int empty_chunks = 0;
    while (cpu.get_thread_time() < time_per_test)
    {
      runs++;
      f->reset();
      f->process(&ichunk);
      while (!f->is_empty())
      {
        f->get_chunk(&ochunk);
        if (ochunk.size)
          data_chunks++;
        else
          empty_chunks++;
      }
    }
    cpu.stop();

    log->msg("Detector speed on noise: %iMB/s, Data: %i, Empty: %i", 
      int(double(noise_size) * runs / cpu.get_thread_time() / 1000000), 
      data_chunks / runs, empty_chunks / runs);
  }

};

///////////////////////////////////////////////////////////////////////////////
// Test function

int test_detector(Log *log)
{
  Detector_test test(log);
  return test.test();
}
