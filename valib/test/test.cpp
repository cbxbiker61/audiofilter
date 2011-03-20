#include <cstdlib>
#include <cstdio>
#include "suite.h"


// Common tests

EXTERN_SUITE(suite_test);

EXTERN_SUITE(general);
EXTERN_TEST(rng);
EXTERN_SUITE(bitstream);
EXTERN_SUITE(base);
EXTERN_SUITE(fir);
EXTERN_SUITE(linear_filter);
EXTERN_TEST(cache);


EXTERN_TEST(slice);
EXTERN_TEST(convolver);
EXTERN_TEST(convolver_mch);
EXTERN_SUITE(resample);
EXTERN_SUITE(proc);
EXTERN_TEST(old_style);

// Heavy tests

EXTERN_TEST(rng_proof);

// Speed tests

///////////////////////////////////////////////////////////
// Common tests

FLAT_SUITE(tests, "Common tests")
  SUITE_FACTORY(suite_test),
  SUITE_FACTORY(general),
   TEST_FACTORY(rng),
  SUITE_FACTORY(bitstream),
  SUITE_FACTORY(base),
  SUITE_FACTORY(fir),
  SUITE_FACTORY(linear_filter),
   TEST_FACTORY(cache),
   TEST_FACTORY(slice),
   TEST_FACTORY(convolver),
   TEST_FACTORY(convolver_mch),
  SUITE_FACTORY(resample),
  SUITE_FACTORY(proc),

   TEST_FACTORY(old_style),
SUITE_END;

///////////////////////////////////////////////////////////
// Heavy tests

FLAT_SUITE(heavy, "Heavy tests")
  TEST_FACTORY(rng_proof),
SUITE_END;

///////////////////////////////////////////////////////////
// Speed tests

FLAT_SUITE(speed, "Speed tests")
SUITE_END;

FLAT_SUITE(all, "All tests")
  SUITE_FACTORY(tests),
  SUITE_FACTORY(heavy),
  SUITE_FACTORY(speed),
SUITE_END;

///////////////////////////////////////////////////////////////////////////////
// Main
///////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
  Log log(LOG_SCREEN | LOG_HEADER | LOG_STATUS, "test.log");
  log.msg("Valib build info:\n%s", valib_build_info());

  Test *all = CREATE_SUITE(all);

  if (argc == 1)
  {
    // Run common tests only
    Test *t = all->find("tests");
    t->run(&log);
  }

  if (argc > 1) for (int i = 1; i < argc; i++)
  {
    Test *t = all->find(argv[i]);
    if (t == 0)
      log.err("Unknown test: %s", argv[i]);
    else
      t->run(&log);
  }

  delete all;

  log.msg("-----------------------------------------------------------");

  int total_time = (int)log.get_total_time();
  log.msg("Total time: %i:%02i", total_time / 60, total_time % 60);
  if (log.get_total_errors())
    log.msg("There are %i errors!\n", log.get_total_errors());
  else
    log.msg("Ok!");

  return log.get_total_errors();
}
 

/*
extern int test_ac3_parser_compare(const char *filename, const char *desc);
int test_ac3_parser()
{
  int err = 0;
  printf("\n* AC3Parser test (compare with LibA52)\n");
  err += test_ac3_parser_compare("f:\\ac3\\ac3test.ac3", "general 5.1");
  err += test_ac3_parser_compare("f:\\ac3\\maria.ac3",   "stereo");
  err += test_ac3_parser_compare("f:\\ac3\\surraund.ac3","Dolby surround");
  return err;
}

extern int test_ac3_enc(Log *log, const char *_raw_filename, const char *_desc, Speakers _spk, int _bitrate, int _nframes);
int test_ac3_enc_all(Log *log)
{
  log->open_group("AC3Enc test");
  test_ac3_enc(log, "f:\\ac3\\ac3test_pcm16.raw", "stereo, 448kbps", Speakers(FORMAT_PCM16, MODE_STEREO, 48000, 65535), 448000, 1875);
  test_ac3_enc(log, "f:\\ac3\\ac3test_pcm16_6ch.raw", "5.1, 448kbps", Speakers(FORMAT_PCM16, MODE_5_1, 48000, 65535), 448000, 1874);
  return log->close_group();
}
*/
