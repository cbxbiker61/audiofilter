#ifndef SUITE_H
#define SUITE_H

#include <string>
#include <vector>
#include "log.h"
#include "filter.h"
#include "vtime.h"

///////////////////////////////////////////////////////////////////////////////
// Compare funcitons
///////////////////////////////////////////////////////////////////////////////

// Compare two streams from two sources. Stream formats are not compared when
// reference stream has FORMAT_RAWDATA format. In this case data is compared
// bonary, so formats of both streams must be raw, not FORMAT_LINEAR.
// Second form of compare() uses filters to process streams.
// Third form of compare() uses files instead of sources.

int compare(Log *log, Source *src, Source *ref);
int compare(Log *log, Source *src, Filter *src_filter, Source *ref, Filter *ref_filter = 0);
int compare_file(Log *log, Speakers spk_src, const char *fn_src, Filter *src_filter, const char *fn_ref);

sample_t calc_peak(Source *s);
sample_t calc_peak(Source *s, Filter * f);

double calc_rms(Source *s);
double calc_rms(Source *s, Filter * f);

sample_t calc_diff(Source *s1, Source *s2);
sample_t calc_diff(Source *s1, Filter *f1, Source *s2, Filter *f2);

double calc_rms_diff(Source *s1, Source *s2);
double calc_rms_diff(Source *s1, Filter *f1, Source *s2, Filter *f2);

///////////////////////////////////////////////////////////////////////////////
// Test class is the base for all tests and suites
///////////////////////////////////////////////////////////////////////////////

class Test;
class TestResult;
typedef Test *test_factory();

class TestResult
{
protected:
  bool result;

public:
  TestResult(bool r): result(r) {};
  TestResult(const TestResult &r) { result = r.result; }
  TestResult &operator =(const TestResult &r) { result = r.result; return *this; }

  bool passed() { return result; }
  bool failed() { return !result; }
};

extern const TestResult test_passed;
extern const TestResult test_failed;

class Test
{
protected:
  std::string name;
  std::string label;
  bool flat;
  vtime_t runtime;
  Log *log;

  virtual TestResult do_test() = 0;

public:
  Test(const char *_name, const char *_label, bool _flat = true): name(_name), label(_label), flat(_flat), runtime(0), log(0) {}
  virtual ~Test() {}

  virtual Test *find(const char *_name)
  {
    return name == _name? this: 0;
  }

  virtual TestResult run(Log *_log)
  {
    assert(_log);
    log = _log;

    if (flat)
      log->msg(label.c_str());
    else
      log->open_group(label.c_str());

    runtime = local_time();
    TestResult result = do_test();
    runtime = local_time() - runtime;

    if (!flat)
      log->close_group();

    log = 0;
    return result;
  }

  virtual vtime_t time() const { return runtime; }

  virtual const char *get_name() const { return name.c_str(); }
  virtual const char *get_label() const { return label.c_str(); }
};

///////////////////////////////////////////////////////////////////////////////
// TestSuite is the base for test suites (sets of tests)
///////////////////////////////////////////////////////////////////////////////

class TestSuite : public Test
{
protected:
  std::vector<Test *> tests;

  virtual TestResult do_test()
  {
    bool passed = true;
    for (size_t i = 0; i < tests.size(); i++)
    {
      // Here we assume that tests are indepenent, so we can continure testing
      // even after fail. In this way we can see a full list of failed
      // modules. It is important because a module that causes the trouble may
      // be tested AFTER modules that depend on it.
      TestResult result = tests[i]->run(log).passed();
      passed = passed && result.passed();
    }
    return TestResult(passed);
  }

public:
  TestSuite(const char *_name, const char *_label, bool _flat, test_factory *_tests[]): Test(_name, _label, _flat)
  {
    assert(_tests);
    for (int i = 0; _tests[i] != 0; i++)
    {
      Test *test = _tests[i]();
      tests.push_back(test);
    }
  }
  ~TestSuite()
  {
    for (size_t i = 0; i < tests.size(); i++)
      delete tests[i];
  }

  virtual Test *find(const char *_name)
  {
    if (name == _name) return this;

    for (size_t i = 0; i < tests.size(); i++)
    {
      Test *t = tests[i]->find(_name);
      if (t) return t;
    }

    return 0;
  }

  size_t ntests() const { return tests.size(); }
};

///////////////////////////////////////////////////////////////////////////////
// Test macros
// Used to create simple tests (generally just a one function).
// Tests are created using a test factory function. To get a pointer to that
// function use TEST_FACTORY function (factories may be used in test lists, 
// suites, etc). If you want just to create a test use CREATE_TEST macro.
// 
// Example:
// ========
//
// TEST(some_test, "Test something important")
//   int var = 1;
//   CHECK(var == 1);
//   ...
// TEST_END;
//
//
// TestResult big_test(Log *log)
// {
//   ...
//   Test *t = CREATE_TEST(some_test);
//   CHECKT(t->run(log).passed(), ("Cannot continue: %s failed!", t->name()));
//   ...
//   // here we can do something that depends on some_test's success
// }
//
// test_factory *array_of_tests[] = {
//   TEST_FACTORY(some_test),
//   TEST_FACTORY(one_more_test),
//   ...
//   0 };
//
// bool run_tests(Log *log, test_factory *list[]) {
//   for (int i = 0; list[i] != 0; i++) {
//     Test *t = list[i]();
//     log->msg(t.get_name());
//     t->run(log); 
//     // Here we assume that tests are indepenent, so we can continure testing
//     // even after fail. In this way we can see a full list of failed
//     // modules. It is important because a module that causes the trouble may
//     // be tested AFTER modules that depend on it.
//   }
// }
//
///////////////////////////////////////////////////////////////////////////////

#define TEST(name, label)                             \
class Test_##name : public Test {                     \
public:                                               \
  Test_##name(): Test(#name, label) {}                \
protected:                                            \
  virtual TestResult do_test() {

#define TEST_END(name)                                \
    return test_passed; }                             \
};                                                    \
extern "C" Test *test_factory_##name()                \
{ return new Test_##name(); };                        \

#define CREATE_TEST(name) test_factory_##name()
#define TEST_FACTORY(name) test_factory_##name
#define EXTERN_TEST(name) extern "C" Test *test_factory_##name()



///////////////////////////////////////////////////////////////////////////////
// Suite macros
// These macros help to create simple list of tests. Suite is just a special
// case of test, that runs all tests listed.
//
// Example:
// ========
//
// SUITE(big_module_test, "Huge amount of tests")
//   TEST_FACTORY(module_test_1),
//   SUITE_FACTORY(module_suite_1),
//   TEST_FACTORY(module_test_2),
//   ...
//   TEST_FACTORY(module_test_n),
// SUITE_END;
//
// bool test_big_module(Log *log) {
//   Test *t = CREATE_SUITE(big_module_test);
//   CHECKT(t.run(), ("Big Module is broken"));
// }
// 
///////////////////////////////////////////////////////////////////////////////

#define TEST_SUITE(name, label, flat)                     \
class Suite_##name : public TestSuite {                   \
public:                                                   \
  static Test *create() { return new Suite_##name(); }    \
  Suite_##name(): TestSuite(#name, label, flat, suite) {} \
protected:                                                \
  static test_factory *suite[];                           \
};                                                        \
extern "C" Test *suite_factory_##name()                   \
{ return new Suite_##name(); }                            \
test_factory *Suite_##name::suite[] = {

#define SUITE_END   0 };

#define SUITE(name, label) TEST_SUITE(name, label, false)
#define FLAT_SUITE(name, label) TEST_SUITE(name, label, true)

#define CREATE_SUITE(name) suite_factory_##name()
#define SUITE_FACTORY(name) suite_factory_##name
#define EXTERN_SUITE(name) extern "C" Test *suite_factory_##name()

///////////////////////////////////////////////////////////////////////////////

#define CHECK(c) { if (!(c)) { log->err("Check failed: %s (%s:%i)", #c, __FILE__, __LINE__); return test_failed; } }
#define CHECKT(c, desc) { if (!(c)) { log->msg("Check failed: %s (%s:%i)", #c, __FILE__, __LINE__); log->err desc; return test_failed; } }

#define CHECK_DELTA(a, b, delta) { if ((a) > (b)? ((a)-(b)) > (delta): ((b)-(a)) > (delta)) { log->err("Check failed: |%s-%s| > %s (%s:%i)", #a, #b, #delta, __FILE__, __LINE__); return test_failed; } }
#define CHECKT_DELTA(a, b, delta, desc) { if ((a) > (b)? ((a)-(b)) > (delta): ((b)-(a)) > (delta)) { log->msg("Check failed: |%s-%s| > %s (%s:%i)", #a, #b, #delta, __FILE__, __LINE__); log->err desc; return test_failed; } }

#endif
