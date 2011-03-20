/*
  FilterGraph test
  * FilterGraph must behave like NullFilter
*/

#include "suite.h"
#include "filter_tester.h"
#include "filter_graph.h"
#include <source\generator.h>
#include <source\raw_source.h>
#include <win32\cpu.h>

static const int seed = 309485;
static const int noise_size = 65536;

///////////////////////////////////////////////////////////////////////////////
// Test class

class FilterGraph_test
{
protected:
  FilterTester t;
  FilterGraph filter_graph;
  Log *log;

public:
  FilterGraph_test(Log *_log)
  :filter_graph(-1)
  {
    log = _log;
    t.link(&filter_graph, log);
  }

  int test()
  {
    log->open_group("FilterGraph test");
    null_transform();
    return log->close_group();
  }

  void null_transform()
  {
    /////////////////////////////////////////////////////////
    // Transform test
    log->open_group("Null transform test");

    Speakers spk;
    NoiseGen src;
    NoiseGen ref;

    // Rawdata test
    log->msg("Rawdata null transform test");
    spk.set(FORMAT_PCM16, MODE_STEREO, 48000);
    src.init(spk, seed, noise_size);
    ref.init(spk, seed, noise_size);

    if (t.set_input(spk))
      compare(log, &src, &t, &ref);
    else
      log->err("Init failed");

    // Linear test
    log->msg("Linear null transform test");
    spk.set(FORMAT_LINEAR, MODE_STEREO, 48000);
    src.init(spk, seed, noise_size);
    ref.init(spk, seed, noise_size);

    if (t.set_input(spk))
      compare(log, &src, &t, &ref);
    else
      log->err("Init failed");

    log->close_group();
  }
};

///////////////////////////////////////////////////////////////////////////////
// Test function

int test_filtergraph(Log *log)
{
  FilterGraph_test test(log);
  return test.test();
}
