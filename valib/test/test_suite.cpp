/*
  Test suite self-test
*/

#include "source/generator.h"
#include "suite.h"

static const int seed = 34570978;
static const int noise_size = 64 * 1024;

///////////////////////////////////////////////////////////////////////////////
// I want to be able to write tests without any additional writing
// (class definition, etc): just say that "I start writing a test here".
// There're two examples of such plain tests below: one always passes and
// another fails. Later we will try to call this tests in different ways.

TEST(plain_pass, "Plain pass test")
  CHECK(true);
TEST_END(plain_pass);

TEST(plain_fail, "Plain failure test")
  CHECK(false);
TEST_END(plain_fail);

///////////////////////////////////////////////////////////////////////////////
// But also I wish to be able to write complex tests with setup, cleanup and
// set of parametrized sub-tests. Here is an example. Also, this test ensures
// that the test function actually runs. We cannot do this with plain test
// because it does not keep any state after completition.

class WasRun : public Test
{
protected:
  bool was_run_flag;

  TestResult do_test()
  {
    CHECK(sub_test1().passed());
    for (int i = 0; i < 3; i++)
      CHECK(sub_test2(i).passed());

    was_run_flag = true;
    return test_passed;
  }

  TestResult sub_test1()
  {
    log->msg("Subtest 1");
    CHECK(true);
    return test_passed;
  }

  TestResult sub_test2(int param)
  {
    log->msg("Subtest 2 with parameter %i", param);
    CHECK(true);
    return test_passed;
  }

public:
  WasRun(): Test("was_run", "Test example"), was_run_flag(false)
  {}

  bool was_run() const { return was_run_flag; }
};

extern "C" Test *was_run_factory() { return new WasRun(); }

///////////////////////////////////////////////////////////////////////////////
// Well, now it's time to collect tests into the suite.
// Suite uses test factories to create test objects. For plain test deafult
// factory can be obtained using TEST_FACTORY

SUITE(suite_pass, "Suite with successive tests only")
  TEST_FACTORY(plain_pass),
  was_run_factory,
SUITE_END;

SUITE(suite_fail, "Suite with fail test")
  TEST_FACTORY(plain_pass),
  TEST_FACTORY(plain_fail),
SUITE_END;

///////////////////////////////////////////////////////////////////////////////
// Test suite self-test

TEST(suite_test, "Self-test")
  Test *test;
  Test *suite;
  test_factory *f;

  // I don't want to pollute the main log with results of
  // test calls, so I need a dummy log with no output

  Log dummy_log(0);

  /////////////////////////////////////////////////////////
  // Ensure that a test actualy runs

  WasRun was_run;
  CHECK(was_run.was_run() == false);
  CHECK(was_run.run(&dummy_log).passed());
  CHECK(was_run.was_run() == true);

  /////////////////////////////////////////////////////////
  // Run plain tests (pass and fail)
  // Create tests using CREATE_TEST (only for plain tests)

  test = CREATE_TEST(plain_pass);
  CHECK(test->run(&dummy_log).passed());
  delete test;

  test = CREATE_TEST(plain_fail);
  CHECK(test->run(&dummy_log).failed());
  delete test;

  /////////////////////////////////////////////////////////
  // Create a test using a test factory

  f = TEST_FACTORY(plain_pass);
  test = f();
  CHECK(test->run(&dummy_log).passed());
  delete test;

  /////////////////////////////////////////////////////////
  // Call tests from a suite
  // Create a suite using CREATE_SUITE

  suite = CREATE_SUITE(suite_pass);
  CHECK(suite->run(&dummy_log).passed());
  delete suite;

  suite = CREATE_SUITE(suite_fail);
  CHECK(suite->run(&dummy_log).failed());
  delete suite;

  /////////////////////////////////////////////////////////
  // Create a suite using a suite factory

  f = SUITE_FACTORY(suite_pass);
  suite = f();
  CHECK(suite->run(&dummy_log).passed());
  delete suite;

  /////////////////////////////////////////////////////////
  // Search a test in a suite

  suite = CREATE_SUITE(suite_fail);
  test = suite->find("suite_fail"); // find the suite itself
  CHECK(test == suite);

  test = suite->find("plain_pass");
  CHECK(test != 0);
  CHECK(test->run(&dummy_log).passed());

  test = suite->find("plain_fail");
  CHECK(test != 0);
  CHECK(test->run(&dummy_log).failed());

  test = suite->find("fake");
  CHECK(test == 0);
  delete suite;

TEST_END(suite_test);

///////////////////////////////////////////////////////////////////////////////
// compare() funciton test

TEST(suite_compare, "compare() test")

  Speakers linear_spk(FORMAT_LINEAR, MODE_STEREO, 48000);
  Speakers rawdata_spk(FORMAT_PCM16, MODE_STEREO, 48000);
  NoiseGen src_noise;
  NoiseGen ref_noise;
  Log dummy_log(0);

  /////////////////////////////////////////////////////////
  // Linear format test

  src_noise.init(linear_spk, seed, noise_size);
  ref_noise.init(linear_spk, seed, noise_size);
  CHECK(compare(&dummy_log, &src_noise, &ref_noise) == 0);

  src_noise.init(linear_spk, seed, noise_size);
  ref_noise.init(linear_spk, seed + 1, noise_size);
  CHECK(compare(&dummy_log, &src_noise, &ref_noise) > 0);

  /////////////////////////////////////////////////////////
  // Rawdata format test

  src_noise.init(rawdata_spk, seed, noise_size);
  ref_noise.init(rawdata_spk, seed, noise_size);
  CHECK(compare(&dummy_log, &src_noise, &ref_noise) == 0);

  src_noise.init(rawdata_spk, seed, noise_size);
  ref_noise.init(rawdata_spk, seed + 1, noise_size);
  CHECK(compare(&dummy_log, &src_noise, &ref_noise) > 0);

  /////////////////////////////////////////////////////////
  // Length test

  src_noise.init(rawdata_spk, seed, noise_size * 2);
  ref_noise.init(rawdata_spk, seed, noise_size);
  CHECK(compare(&dummy_log, &src_noise, &ref_noise) > 0);

  src_noise.init(rawdata_spk, seed, noise_size);
  ref_noise.init(rawdata_spk, seed, noise_size * 2);
  CHECK(compare(&dummy_log, &src_noise, &ref_noise) > 0);

TEST_END(suite_compare);

///////////////////////////////////////////////////////////////////////////////
// Peak and RMS functions test

TEST(suite_level, "Peak and RMS functions test")

  Speakers spk(FORMAT_LINEAR, MODE_STEREO, 48000);
  NoiseGen noise;
  ZeroGen  zero;
  double level;

  // Noise level
  noise.init(spk, seed, noise_size);
  level = calc_peak(&noise);
  CHECK(level > 0.9);

  // Zero level
  zero.init(spk, noise_size);
  level = calc_peak(&zero);
  CHECK(level == 0.0);

  // Noise RMS ~= -4.77dB +- 1dB
  noise.init(spk, seed, noise_size);
  level = calc_rms(&noise);
  CHECK(level > 0.5146 && level < 0.6479);

  // Zero RMS
  zero.init(spk, noise_size);
  level = calc_rms(&zero);
  CHECK(level == 0.0);

TEST_END(suite_level);

///////////////////////////////////////////////////////////////////////////////
// Difference functions test

TEST(suite_diff, "Difference functions test")

  Speakers spk(FORMAT_LINEAR, MODE_STEREO, 48000);
  NoiseGen noise1;
  NoiseGen noise2;
  ZeroGen  zero;

  double diff;
  double level;

  // diff(noise, noise) == 0
  noise1.init(spk, seed, noise_size);
  noise2.init(spk, seed, noise_size);
  diff = calc_diff(&noise1, &noise2);
  CHECK(diff == 0.0);

  // diff(noise1, noise2) != 0
  noise1.init(spk, seed, noise_size);
  noise2.init(spk, seed + 100, noise_size);
  diff = calc_diff(&noise1, &noise2);
  CHECK(diff > 1.8);

  // diff(noise, zero) == level(noise)
  noise1.init(spk, seed, noise_size);
  noise2.init(spk, seed, noise_size);
  zero.init(spk, noise_size);
  diff = calc_diff(&noise1, &zero);
  level = calc_peak(&noise2);
  CHECK(diff == level);

  // rms_diff(noise, noise) == 0
  noise1.init(spk, seed, noise_size);
  noise2.init(spk, seed, noise_size);
  diff = calc_rms_diff(&noise1, &noise2);
  CHECK(diff == 0.0);

  // rms_diff(noise1, noise2) == -1.77dB +- 1dB
  noise1.init(spk, seed, noise_size);
  noise2.init(spk, seed + 100, noise_size);
  diff = calc_rms_diff(&noise1, &noise2);
  CHECK(diff > 0.7269 && diff < 0.9151);

  // rms_diff(noise, zero) = rms(noise)
  noise1.init(spk, seed, noise_size);
  noise2.init(spk, seed, noise_size);
  zero.init(spk, noise_size);
  diff = calc_rms_diff(&noise1, &zero);
  level = calc_rms(&noise2);
  CHECK(diff == level);

TEST_END(suite_diff);

///////////////////////////////////////////////////////////////////////////////
// Test suite

SUITE(suite_test, "Test suite self-test")
  TEST_FACTORY(suite_test),
  TEST_FACTORY(suite_compare),
  TEST_FACTORY(suite_level),
  TEST_FACTORY(suite_diff),
SUITE_END;
