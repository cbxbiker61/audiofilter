/*
  Test some general assumptions
  (test entities defined at defs.h)

  * Simple IEEE floating point format compatibility test
  * Test size of types
  * Byte swap functions test
  * Byteorder test
*/

#include <stdio.h>
#include "defs.h"
#include "../suite.h"

inline int32_t sample_mant(sample_t s);
inline int16_t sample_exp(sample_t s);

///////////////////////////////////////////////////////////////////////////////
// Simple IEEE floating point format compatibility test

static int test_one_float(Log *log, sample_t s, int32_t mant, int16_t exp)
{
  if (sample_mant(s) != mant || sample_exp(s) != exp)
  {
    log->err("s = %0.4f; mant = %.4f (%.4f), exp = %i (%i)", 
      s, 
      double(sample_mant(s))/2147483648.0,
      double(mant)/2147483648.0,
      sample_exp(s),
      exp);
    return 1;
  }
  return 0;
}

TEST(general_float, "Floating-point compatibility test")
  test_one_float(log, +2.00, +0x40000000, +2);
  test_one_float(log, -2.00, -0x40000000, +2);

  test_one_float(log, +1.00, +0x40000000, +1);
  test_one_float(log, -1.00, -0x40000000, +1);

  test_one_float(log, +0.50, +0x40000000, 0);
  test_one_float(log, -0.50, -0x40000000, 0);

  test_one_float(log, +0.25, +0x40000000, -1);
  test_one_float(log, -0.25, -0x40000000, -1);

  test_one_float(log, +0.125, +0x40000000, -2);
  test_one_float(log, -0.125, -0x40000000, -2);

  test_one_float(log, +0.75, +0x60000000, 0);
  test_one_float(log, -0.75, -0x60000000, 0);
TEST_END(general_float);

///////////////////////////////////////////////////////////////////////////////
// Test size of types

TEST(general_types, "Size of types test")
  if (sizeof(int32_t)  != 4) log->err("sizeof(int32_t) != 4");
  if (sizeof(uint32_t) != 4) log->err("sizeof(uint32_t) != 4");

  if (sizeof(int24_t)  != 3) log->err("sizeof(int24_t) != 3");

  if (sizeof(int16_t)  != 2) log->err("sizeof(int16_t) != 2");
  if (sizeof(uint16_t) != 2) log->err("sizeof(uint16_t) != 2");

  if (sizeof(int8_t)   != 1) log->err("sizeof(int8_t) != 1");
  if (sizeof(uint8_t)  != 1) log->err("sizeof(uint8_t) != 1");

  if (sizeof(float)    != 4) log->err("sizeof(float) != 4");
TEST_END(general_types);

///////////////////////////////////////////////////////////////////////////////
// Swab functions test

TEST(general_swab, "Swab functions test")
  if (swab_u32(0x01020304) != 0x04030201) log->err("swab_u32() does not work");
  if (swab_s32(0x01020304) != 0x04030201) log->err("swab_s32() does not work");
  if (swab_s24(0x010203)   != 0x030201)   log->err("swab_s24() does not work");
  if (swab_u16(0x0102)     != 0x0201)     log->err("swab_u16() does not work");
  if (swab_s16(0x0102)     != 0x0201)     log->err("swab_s16() does not work");
TEST_END(general_swab);

///////////////////////////////////////////////////////////////////////////////
// Byteorder test

TEST(general_byteorder, "Byteorder test");

  const char *buf = "\01\02\03\04\05\06\07\08";

  int32_t  *pi32  = (int32_t *)buf;
  uint32_t *pui32 = (uint32_t *)buf;

  int24_t  *pi24  = (int24_t *)buf;

  int16_t  *pi16  = (int16_t *)buf;
  uint16_t *pui16 = (uint16_t *)buf;

  ///////////////////////////////////////////////////////
  // Determine machine endian

  if (*pui32 == 0x01020304)
    log->msg("Machine uses big endian (Motorola)");
  else if (*pui32 == 0x04030201)
    log->msg("Machine uses little endian (Intel)");
  else
    log->msg("Unknown machine type");

  ///////////////////////////////////////////////////////
  // Low endian read test (le2intxx functions)

  if (le2int32(*pi32)   != 0x04030201) log->err("le2int32() does not work");
  if (le2uint32(*pui32) != 0x04030201) log->err("le2uint32() does not work");
  if (le2int24(*pi24)   != 0x030201)   log->err("le2int24() does not work");
  if (le2int16(*pi16)   != 0x0201)     log->err("le2int16() does not work");
  if (le2uint16(*pui16) != 0x0201)     log->err("le2uint16() does not work");

  ///////////////////////////////////////////////////////
  // Big endian read test (be2intxx functions)

  if (be2int32(*pi32)   != 0x01020304) log->err("be2int32() does not work");
  if (be2uint32(*pui32) != 0x01020304) log->err("be2uint32() does not work");
  if (be2int24(*pi24)   != 0x010203)   log->err("be2int24() does not work");
  if (be2int16(*pi16)   != 0x0102)     log->err("le2int16() does not work");
  if (be2uint16(*pui16) != 0x0102)     log->err("le2uint16() does not work");

  ///////////////////////////////////////////////////////
  // Low endian write test (int2lexx functions)

  if (int2le32(0x04030201)  != *pi32)  log->err("int2le32() does not work");
  if (uint2le32(0x04030201) != *pui32) log->err("uint2le32() does not work");
  if (int2le24(0x030201)    != *pi24)  log->err("int2le24() does not work");
  if (int2le16(0x0201)      != *pi16)  log->err("int2le16() does not work");
  if (uint2le16(0x0201)     != *pui16) log->err("uint2le16() does not work");

  ///////////////////////////////////////////////////////
  // Big endian write test (int2bexx functions)

  if (int2be32(0x01020304)  != *pi32)  log->err("int2be32() does not work");
  if (uint2be32(0x01020304) != *pui32) log->err("uint2be32() does not work");
  if (int2be24(0x010203)    != *pi24)  log->err("int2be24() does not work");
  if (int2be16(0x0102)      != *pi16)  log->err("int2be16() does not work");
  if (uint2be16(0x0102)     != *pui16) log->err("uint2be16() does not work");

TEST_END(general_byteorder);

///////////////////////////////////////////////////////////////////////////////
// Test suite

SUITE(general, "General tests")
  TEST_FACTORY(general_float),
  TEST_FACTORY(general_types),
  TEST_FACTORY(general_swab),
  TEST_FACTORY(general_byteorder),
SUITE_END;

///////////////////////////////////////////////////////////////////////////////
// sample_t utils
///////////////////////////////////////////////////////////////////////////////

#ifndef FLOAT_SAMPLE

  union double_t 
  { 
    double d;
    uint8_t raw[8];
    struct 
    {
      unsigned mant_low  : 32;
      unsigned mant_high : 20;
      unsigned exp       : 11;
      unsigned sign      : 1;
    };

    double_t(double _d) { d = _d;   }
    operator double &() { return d; }
  };

  inline int32_t sample_mant(sample_t s)
  {
    uint32_t mant;
    double_t d = s;

    mant = 0x40000000 | (d.mant_high << 10) | (d.mant_low >> 22);
    mant = (mant ^ (unsigned)(-(int)d.sign)) + d.sign;
    return (int32_t)mant;
  }

  inline int16_t sample_exp(sample_t s)
  {
    double_t d = s;
    return int16_t(d.exp) - 1023 + 1;
  }

#else // #ifndef FLOAT_SAMPLE

  union float_t
  {
    float f;
    struct 
    {
      unsigned mantissa:23;
      unsigned exponent:8;
      unsigned sign:1;
    };
  };


  inline int32_t sample_mant(sample_t s);
  {
    int32_t i32;
    i32 = float_t(s).mant_high;
    (uint32_t)i32 |= float_t(s).sign << 31;
    return i32;
  }

  inline int sample_exp(sample_t s)
  {
    return float_t(s).exp;
  }

#endif // #ifndef FLOAT_SAMPLE ... #else ... 
