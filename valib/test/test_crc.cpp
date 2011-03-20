/*
  Test CRC class

  * Test CRC correctness for bitstream and bytestream interface using 
    2 standard polinomials (crc16 and crc32), different bitstream types, 
    message lengths and shifts
  * Test CRC of large noise buffer with different bitstreams
  * Test CRC speed of different bitstreams
  * Test speed of simple CRC16 table algorithm
*/

#include "log.h"
#include "crc.h"
#include "rng.h"
#include "source\raw_source.h"
#include "win32\cpu.h"

static const vtime_t time_per_test = 1.0; // 1 sec for each speed test
static const int size = 10000000;         // use 10MB noise buffer
static const int err_dist = 12689;        // generate error each N bytes


///////////////////////////////////////////////////////////////////////////////
// Test class
///////////////////////////////////////////////////////////////////////////////

class CRCTest
{
protected:
  Log *log;
  CRC crc;

public:
  CRCTest(Log *_log)
  {
    log = _log;
  }

  int test()
  {
    log->open_group("CRC test");
    math_test();
    speed_test();
    return log->close_group();
  }

  ///////////////////////////////////////////////////////////////////////////////
  // Test math

  void math_test()
  {
    log->open_group("CRC math test");
    bytestream_test(POLY_CRC16, 16, "CRC16");
    bytestream_test(POLY_CRC32, 32, "CRC32");
    bitstream_test(POLY_CRC16, 16, "CRC16");
    bitstream_test(POLY_CRC32, 32, "CRC32");
    log->close_group();
  }

  ///////////////////////////////////////////////////////////////////////////////
  // Test speed of all bitstream types

  void speed_test()
  {
    RNG rng(47564321);
    uint8_t *data = new uint8_t[size];
    rng.fill_raw(data, size);

    crc.init(POLY_CRC16, 16);

    log->open_group("CRC speed test");
    speed_test_table(data, size, 0xc30d);
    speed_test(data, size, 0xc30d0000);
    log->close_group();

    delete data;
  }

  ///////////////////////////////////////////////////////////////////////////////
  // Test bytestream interface
  // * test different message lengths
  // * test different message shifts

  int bytestream_test(int poly, int power, const char *poly_name)
  {
    static const int max_size = 16;
    static const int max_shift = 16;

    int i;
    RNG rng;
    uint8_t buf[max_size + max_shift + 1];
    uint32_t crc_msg;
    uint32_t crc_test;

    log->msg("Bytestream test with %s polinomial", poly_name);

    crc.init(poly, power);
    for (int size = 0; size < max_size; size++)
      for (int shift = 0; shift < max_shift; shift++)
      {
        rng.fill_raw(buf, sizeof(buf));

        // calc message reference crc
        crc_msg = 0;
        for (i = 0; i < size; i++)
          crc_msg = crc.add_bits(crc_msg, buf[shift + i], 8);

        // calc message test crc
        crc_test = crc.calc(0, buf + shift, size);

        // test it
        if (crc_test != crc_msg)
          return log->err("bitstream: size = %i, shift: %i, crc = 0x%x (must be 0x%x)", 
            size, shift, crc_test, crc_msg);
      }
    return 0;
  }

  ///////////////////////////////////////////////////////////////////////////////
  // Test bitstream interface
  // * test different bitstream types
  // * test different message lengths
  // * test different message shifts

  int bitstream_test(int poly, int power, const char *poly_name)
  {
    static const int max_size = 16*8;  // in bits!!!
    static const int max_shift = 16*8; // in bits!!!

    int i;
    RNG rng;
    uint8_t buf[(max_size + max_shift) / 8 + 1];
    uint32_t crc_msg;
    uint32_t crc_test;

    log->msg("Bitstream test with %s polinomial", poly_name);

    crc.init(poly, power);
    for (int size = 0; size < max_size; size++)
      for (int shift = 0; shift < max_shift; shift++)
      {
        int start_byte = shift / 8;
        int start_bit  = shift % 8;
        int end_byte   = (shift + size) / 8;
        int end_bit    = (shift + size) % 8;

        rng.fill_raw(buf, sizeof(buf));

        // calc message reference crc
        crc_msg = 0;
        if (start_byte == end_byte)
          crc_msg = crc.add_bits(crc_msg, buf[start_byte] >> (8 - end_bit), size);
        else
        {
          crc_msg = crc.add_bits(crc_msg, buf[start_byte], 8 - start_bit);
          for (i = start_byte + 1; i < end_byte; i++)
            crc_msg = crc.add_bits(crc_msg, buf[i], 8);
          crc_msg = crc.add_bits(crc_msg, buf[end_byte] >> (8 - end_bit), end_bit);
        }

        // calc message test crc
        crc_test = 0;
        crc_test = crc.calc_bits(crc_test, buf, shift, size);

        // test it
        if (crc_test != crc_msg)
          return log->err("bitstream: size = %i, shift: %i, crc = 0x%x (must be 0x%x)", 
            size, shift, crc_test, crc_msg);
      }
    return 0;
  }

  ///////////////////////////////////////////////////////////////////////////////
  // Simple table algorithm speed test

  int speed_test_table(uint8_t *data, size_t size, uint32_t crc_test)
  {
    // table method speed test
    uint32_t result = 0;
    CPUMeter cpu;

    int runs = 0;
    cpu.start();
    while (cpu.get_thread_time() < time_per_test)
    {
      runs++;
      result = calc_crc(0, data, size);
    }
    cpu.stop();

    log->msg("CRC speed (table method): %iMB/s",
      int(double(size) * runs / cpu.get_thread_time() / 1000000));

    if (result != crc_test)
      log->err("crc = 0x%08x but must be 0x%08x", result, crc_test);

    return log->get_errors();
  }

  ///////////////////////////////////////////////////////////////////////////////
  // Speed test
  // Just calc CRC of large block

  int speed_test(uint8_t *data, size_t size, uint32_t crc_test)
  {
    uint32_t result = 0;
    CPUMeter cpu;

    int runs = 0;
    cpu.start();
    while (cpu.get_thread_time() < time_per_test)
    {
      runs++;
      result = crc.calc(0, data, size);
    }
    cpu.stop();

    log->msg("CRC speed: %iMB/s",
      int(double(size) * runs / cpu.get_thread_time() / 1000000));

    if (result != crc_test)
      log->err("crc = 0x%08x but must be 0x%08x", result, crc_test);

    return log->get_errors();
  }

};


///////////////////////////////////////////////////////////////////////////////
// Test function
///////////////////////////////////////////////////////////////////////////////

int test_crc(Log *log)
{
  CRCTest test(log);
  return test.test();
}
