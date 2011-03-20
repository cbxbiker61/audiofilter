/*
  Bitstream test
  (classes defined at bitstream.h)

  * ReadBS
  * WriteBS

*/

#include <math.h>
#include "bitstream.h"
#include "auto_buf.h"
#include "rng.h"
#include "../suite.h"

static const int seed = 4796;
static const int noise_size = 64 * 1024;

static uint32_t get_uint32(uint8_t *buf, size_t start_bit)
{
  unsigned shift = start_bit & 7;
  uint32_t dword = be2uint32(*(uint32_t *)(buf + start_bit / 8));
  uint8_t next_byte = *(buf + start_bit / 8 + 4);
  return (dword << shift) | (next_byte >> (8 - shift));
}

///////////////////////////////////////////////////////////////////////////////
// ReadBS test
///////////////////////////////////////////////////////////////////////////////

TEST(bitstream_read, "ReadBS")
  ReadBS bs;
  uint32_t result, test;

  const size_t max_align = 64;
  const size_t max_start_bit = 64;

  AutoBuf<uint8_t> buf(max_align + max_start_bit / 8 + 4);

  RNG rng(seed);
  rng.fill_raw(buf, buf.size());

  /////////////////////////////////////////////////////////
  // ReadBS::set()
  // Test different start positions
  //
  // Start position consists of the buffer pointer and
  // start_bit. Note that ReadBS aligns the pointer, so we
  // have to test different pointer alignments.

  {
    for (uint8_t *pos = buf; pos < buf + max_align; pos++)
      for (size_t start_bit = 0; start_bit < max_start_bit; start_bit++)
      {
        bs.set(pos, start_bit, 32);
        result = bs.get(32);
        test = get_uint32(pos, start_bit);
        CHECKT(result == test, ("bs.set(pos=%i, start_bit=%i) fails", pos-buf, start_bit));
      }
  }

  /////////////////////////////////////////////////////////
  // ReadBS::set_bit_pos()

  {
    for (size_t pos_bits = 0; pos_bits < max_start_bit; pos_bits++)
    {
      bs.set(buf, 0, max_start_bit + 32);
      bs.set_pos_bits(pos_bits);
      result = bs.get(32);
      test = get_uint32(buf, pos_bits);
      CHECKT(result == test, ("bs.set_bit_pos(%i) fails", pos_bits));
    }
  }

  /////////////////////////////////////////////////////////
  // ReadBS::get_bit_pos()
  // Bit position is calculated from all of the following:
  // start pointer, current pointer, start_bit and bits_left.
  // Therefore we have to check it all...

  {
    for (uint8_t *pos = buf; pos < buf + max_align; pos++)
      for (size_t start_bit = 0; start_bit < max_start_bit; start_bit++)
        for (size_t pos_bits = 0; pos_bits <= 32; pos_bits++)
        {
          bs.set(pos, start_bit, 32);
          bs.set_pos_bits(pos_bits);
          size_t test_pos_bits = bs.get_pos_bits();
          CHECKT(test_pos_bits == pos_bits, ("bs.get_pos_bits() fails. pos=%i start_bit=%i pos_bits=%i", pos-buf, start_bit, pos_bits));
        }
  }

  /////////////////////////////////////////////////////////
  // ReadBS::get()
  // Test all number of bits

  {
    bs.set(buf, 0, 32);
    result = bs.get(0);
    CHECKT(result == 0, ("bs.get(%i) fails", 0));

    test = be2uint32(*(uint32_t*)buf.data());
    for (unsigned num_bits = 1; num_bits <= 32; num_bits++)
    {
      bs.set(buf, 0, 32);
      result = bs.get(num_bits);
      CHECKT(result == (test >> (32 - num_bits)), ("bs.get(%i) fails", num_bits));
    }
  }

  /////////////////////////////////////////////////////////
  // ReadBS::get()
  // Test all combinations of num_bits and bits_left

  {
    for (unsigned i = 0; i <= 32; i++)
    {
      test = get_uint32(buf, i);
      for (unsigned j = 0; j <= 32; j++)
      {
        bs.set(buf, 0, 64);
        result = bs.get(i);
        result = bs.get(j);
        if (j == 0)
          CHECKT(result == 0, ("bs.get(%i); bs.get(%i); sequence fails", i, j))
        else
          CHECKT(result == (test >> (32 - j)), ("bs.get(%i); bs.get(%i); sequence fails", i, j));
      }
    }
  }

  /////////////////////////////////////////////////////////
  // ReadBS::get_signed()
  // Test all number of bits
  // Note that we have to test both sings

  {
    bs.set(buf, 0, 64);
    result = bs.get_signed(0);
    CHECKT(result == 0, ("bs.get_signed(%i) fails", 0));

    for (int sign = 0; sign <= 1; sign++)
    {
      *(uint32_t *)buf.data() = ~*(uint32_t *)buf.data();
      int32_t test = be2uint32(*(uint32_t*)buf.data());
      for (unsigned num_bits = 1; num_bits <= 32; num_bits++)
      {
        bs.set(buf, 0, 32);
        result = bs.get_signed(num_bits);
        CHECKT(result == (test >> (32 - num_bits)), ("bs.get_signed(%i) fails", num_bits));
      }
    }
  }

  /////////////////////////////////////////////////////////
  // ReadBS::get_signed()
  // Test all combinations of num_bits and bits_left

  {
    for (int sign = 0; sign <= 1; sign++)
    {
      *(uint32_t *)buf.data() = ~*(uint32_t *)buf.data();
      for (unsigned i = 0; i <= 32; i++)
      {
        int32_t test_signed = get_uint32(buf, i);
        for (unsigned j = 0; j <= 32; j++)
        {
          bs.set(buf, 0, 64);
          result = bs.get_signed(i);
          result = bs.get_signed(j);
          if (j == 0)
            CHECKT(result == 0, ("bs.get(%i); bs.get(%i); sequence fails", i, j))
          else
            CHECKT(result == (test_signed >> (32 - j)), ("bs.get(%i); bs.get(%i); sequence fails", i, j));
        }
      }
    }
  }

TEST_END(bitstream_read);

///////////////////////////////////////////////////////////////////////////////
// WriteBS test
///////////////////////////////////////////////////////////////////////////////

TEST(bitstream_write, "WriteBS")
  WriteBS bs;
  uint32_t result, test;

  const size_t max_align = 64;
  const size_t max_start_bit = 64;

  AutoBuf<uint8_t> buf(max_align + max_start_bit / 8 + 8);
  RNG rng(seed);

  /////////////////////////////////////////////////////////
  // WriteBS::set()
  // Test different start positions
  // Start position consists of the buffer pointer and
  // start_bit. Note that WriteBS aligns the pointer, so we
  // have to test different pointer alignments.

  {
    for (uint8_t *pos = buf; pos < buf + max_align; pos++)
    {
      rng.fill_raw(buf, buf.size());
      for (size_t start_bit = 0; start_bit < max_start_bit; start_bit++)
      {
        test = rng.next();

        bs.set(pos, start_bit, 32);
        bs.put(32, test);
        bs.flush();

        result = get_uint32(pos, start_bit);
        CHECKT(result == test, ("bs.set(pos=%i, start_bit=%i) fails", pos-buf, start_bit));
      }
    }
  }

  /////////////////////////////////////////////////////////
  // WriteBS::set_bit_pos()

  {
    rng.fill_raw(buf, buf.size());
    for (size_t pos_bits = 0; pos_bits < max_start_bit; pos_bits++)
    {
      test = rng.next();

      bs.set(buf, 0, max_start_bit + 32);
      bs.set_pos_bits(pos_bits);
      bs.put(32, test);
      bs.flush();

      result = get_uint32(buf, pos_bits);
      CHECKT(result == test, ("bs.set_pos_bits(%i) fails", pos_bits));
    }
  }

  /////////////////////////////////////////////////////////
  // ReadBS::get_bit_pos()
  // Bit position is calculated from all of the following:
  // start pointer, current pointer, start_bit and bits_left.
  // Therefore we have to check it all...

  {
    for (uint8_t *pos = buf; pos < buf + max_align; pos++)
      for (size_t start_bit = 0; start_bit < max_start_bit; start_bit++)
        for (unsigned pos_bits = 0; pos_bits <= 32; pos_bits++)
        {
          bs.set(pos, start_bit, 32);
          bs.set_pos_bits(pos_bits);
          size_t test_pos_bits = bs.get_pos_bits();
          CHECKT(test_pos_bits == pos_bits, ("bs.get_pos_bits() fails. pos=%i start_bit=%i pos_bits=%i", pos-buf, start_bit, pos_bits));
        }
  }

  /////////////////////////////////////////////////////////
  // WriteBS::put()
  // Test all number of bits

  {
    for (unsigned num_bits = 1; num_bits <= 32; num_bits++)
    {
      *(uint32_t *)buf.data() = rng.next();
      test = rng.next() >> (32 - num_bits);

      bs.set(buf, 0, 32);
      bs.put(num_bits, test);
      bs.flush();

      result = get_uint32(buf, 0) >> (32 - num_bits);
      CHECKT(result == test, ("bs.put(%i) fails", num_bits));
    }
  }

  /////////////////////////////////////////////////////////
  // WriteBS::put()
  // Test all combinations of num_bits and bits_left

  {
    for (unsigned i = 1; i <= 32; i++)
      for (unsigned j = 1; j <= 32; j++)
      {
        *(uint32_t *)buf.data() = rng.next();
        uint32_t test1 = rng.next() >> (32 - i);
        uint32_t test2 = rng.next() >> (32 - j);

        bs.set(buf, 0, 64);
        bs.put(i, test1);
        bs.put(j, test2);
        bs.flush();

        result = get_uint32(buf, i) >> (32 - j);
        CHECKT(result == test2, ("bs.put(%i); bs.put(%i); sequence fails", i, j));
      }
  }

  /////////////////////////////////////////////////////////
  // WriteBS::flush()
  // Data before start_bit must remain unchanged
  // Test different start positions

  {
    for (uint8_t *pos = buf + 4; pos < buf + max_align; pos++)
      for (size_t start_bit = 0; start_bit < max_start_bit; start_bit++)
      {
        rng.fill_raw(buf, buf.size());
        test = get_uint32(pos - 4, start_bit);

        bs.set(pos, start_bit, 32);
        bs.put(32, rng.next());
        bs.flush();

        result = get_uint32(pos - 4, start_bit);
        CHECKT(result == test, ("bs.set(pos=%i, start_bit=%i) changes data before start_bit", pos-buf-4, start_bit));
      }
  }

  /////////////////////////////////////////////////////////
  // WriteBS::flush()
  // Data after the last bit changed must remain unchanged
  // Test all combinations of num_bits and bits_left

  {
    for (unsigned i = 0; i <= 32; i++)
      for (unsigned j = 0; j <= 32; j++)
      {
        rng.fill_raw(buf, buf.size());
        test = get_uint32(buf, i + j);
        uint32_t v1 = (i == 0)? 0: rng.next() >> (32 - i);
        uint32_t v2 = (j == 0)? 0: rng.next() >> (32 - j);

        bs.set(buf, 0, 64);
        bs.put(i, v1);
        bs.put(j, v2);
        bs.flush();

        result = get_uint32(buf, i + j);
        CHECKT(result == test, ("bs.put(%i); bs.put(%i); changes data after the last bit changed", i, j));
      }
  }

  /////////////////////////////////////////////////////////
  // WriteBS::flush()
  // set_pos_bits() must flush at old position

  {
    *(uint32_t *)buf.data() = rng.next();
    test = rng.next();

    bs.set(buf, 0, 64);
    bs.put(32, test);
    bs.set_pos_bits(0);

    result = get_uint32(buf, 0);
    CHECKT(result == test, ("bs.set_pos_bits() does not flush"));
  }



TEST_END(bitstream_write);


///////////////////////////////////////////////////////////////////////////////
// Test suite
///////////////////////////////////////////////////////////////////////////////

SUITE(bitstream, "Bitstream")
  TEST_FACTORY(bitstream_read),
  TEST_FACTORY(bitstream_write),
SUITE_END;
