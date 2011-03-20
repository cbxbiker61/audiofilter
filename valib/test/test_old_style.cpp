#include "suite.h"

int test_bs_convert(Log *log);
int test_crc(Log *log);
int test_syncer(Log *log);
int test_streambuffer(Log *log);
int test_parser_filter(Log *log);

//int test_ac3(Log *log);

int test_rules(Log *log);

int test_crash(Log *log);
int test_demux(Log *log);
int test_spdifer(Log *log);
int test_despdifer(Log *log);
int test_detector(Log *log);

int test_filtergraph(Log *log);
int test_decodergraph(Log *log);
int test_dvdgraph(Log *log);

TEST(old_style, "Old-stlye tests")
  test_rules(log);

  test_crc(log);
  test_syncer(log);
  test_streambuffer(log);
  test_bs_convert(log);
  test_parser_filter(log);
//  test_ac3(log);

  test_crash(log);

  test_demux(log);
  test_spdifer(log);
  test_despdifer(log);
  test_detector(log);

  test_filtergraph(log);
  test_decodergraph(log);
  test_dvdgraph(log);
TEST_END(old_style);
