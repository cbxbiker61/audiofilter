/*
  Bitstream conversion functions test

  Tests:
  * conversion speed
  * conversion compliance (inverse conversion)
  * conversion compliance (3 step conversion)
  * Correct inplace and copy conversion
  * 14bit conversion verification:
    * 2 high bits are zeroed in resulting 14bit stream
    * padding with 8->14bit conversion
    * padding with 14->8bit conversion
*/


#include "log.h"
#include "auto_file.h"
#include "bitstream.h"

#include "rng.h"
#include "win32\cpu.h"

///////////////////////////////////////////////////////////////////////////////
// Test constants

static const vtime_t time_per_test = 1.0; // 1 sec for each speed test
static const int test_size = 500000;      // conversion test buffer size
static const int noise_size = 5000000;    // noise speed test buffer size

///////////////////////////////////////////////////////////////////////////////
// Test class

class BSConvert_test
{
protected:
  Log *log;

  size_t allocated;

  uint8_t* ref_buf;
  size_t ref_size;

  uint8_t* buf1;
  size_t size1;

  uint8_t* buf2;
  size_t size2;

  void prepare(size_t size, int32_t seed)
  {
    if (allocated < size)
    {
      if (ref_buf) delete ref_buf;
      if (buf1)    delete buf1;
      if (buf2)    delete buf2;

      ref_buf = new uint8_t[size];
      buf1    = new uint8_t[size];
      buf2    = new uint8_t[size];

      if (!ref_buf || !buf1 || !buf2)
      {      
        printf("Bitstream conversion test: cannot allocate buffer");
        abort();
      }

      allocated = size;
    }

    RNG rng(seed);
    rng.fill_raw(ref_buf, size);

    ref_size = size;
    size1 = 0;
    size2 = 0;
  }

public:
  BSConvert_test(Log *_log)
  {
    log = _log;

    allocated = 0;
    ref_buf = 0;
    buf1 = 0;
    buf2 = 0;
  }
  ~BSConvert_test()
  {
    if (ref_buf) delete ref_buf;
    if (buf1)    delete buf1;
    if (buf2)    delete buf2;
  }

  int test()
  {
    log->open_group("Bitstream conversion test");

    cycle();
    speed();

    return log->close_group();
  }

  void cycle()
  {
    static const int bs_types[] =
    {
      BITSTREAM_8,
      BITSTREAM_16BE, BITSTREAM_16LE,
      //BITSTREAM_32BE, BITSTREAM_32LE,
      BITSTREAM_14BE, BITSTREAM_14LE
    };

    int i, j, k;

    log->open_group("2-step cycle conversions");
    for (i = 0; i < array_size(bs_types); i++)
      for (j = 0; j < array_size(bs_types); j++)
        if (i != j)
          cycle(bs_types[i], bs_types[j]);
    log->close_group();

    log->open_group("3-step cycle conversions");
    for (i = 0; i < array_size(bs_types); i++)
      for (j = 0; j < array_size(bs_types); j++)
        for (k = 0; k < array_size(bs_types); k++)
          if ((i != j) && (i != k) && (j != k))
            cycle(bs_types[i], bs_types[j], bs_types[k]);
    log->close_group();
  }

  int speed()
  {
    log->open_group("Coversion speed");
    speed(bs_conv_copy,   "(none) ");
    speed(bs_conv_swab16, "swab16 ");
    speed(bs_conv_8_14be, "8->14be");
    speed(bs_conv_8_14le, "8->14le");
    speed(bs_conv_14be_8, "14be->8");
    speed(bs_conv_14le_8, "14le->8");
    speed(bs_conv_14be_16le, "14be->16le");
    speed(bs_conv_14le_16le, "14le->16le");
    return log->close_group();
  }

  int cycle(int bs_type1, int bs_type2, int bs_type3 = -1)
  {
    bs_conv_t conv1 = 0, conv2 = 0, conv3 = 0;

    conv1 = bs_conversion(bs_type1, bs_type2);
    if (!conv1) return log->err("Cannot find conversion %s -> %s", bs_name(bs_type1), bs_name(bs_type2));

    if (bs_type3 == -1)
    {
      conv2 = bs_conversion(bs_type2, bs_type1);
      if (!conv2) return log->err("Cannot find conversion %s -> %s", bs_name(bs_type2), bs_name(bs_type1));

      log->msg("Conversion %s-%s", bs_name(bs_type1), bs_name(bs_type2));
    }
    else
    {
      conv2 = bs_conversion(bs_type2, bs_type3);
      if (!conv2) return log->err("Cannot find conversion %s -> %s", bs_name(bs_type2), bs_name(bs_type3));
      conv3 = bs_conversion(bs_type3, bs_type1);
      if (!conv3) return log->err("Cannot find conversion %s -> %s", bs_name(bs_type3), bs_name(bs_type1));

      log->msg("Conversion %s-%s-%s", bs_name(bs_type1), bs_name(bs_type2), bs_name(bs_type3));
    }

    // Prepare noise block
    prepare(test_size, 4356546);

    // Conversion to 14bit increases amount of data,
    // therefore we must decrease source data size
    if (!is_14bit(bs_type1) && (is_14bit(bs_type2) || is_14bit(bs_type3)))
    {
      ref_size /= 8;
      ref_size *= 7;
    }

    // 14bit source must have even buffer size.
    // To maintain buffer size unchanged it must be multiple of 8.
    // Also we must clear 2 high bits for 14bit stream.
    if (is_14bit(bs_type1))
    {
      ref_size &= ~7;

      size_t n = ref_size;
      if (bs_type1 == BITSTREAM_14LE)
        n++;

      while (n > 1)
      {
        n -= 2;
        ref_buf[n] &= 0x3f;
      }
    }

    // inplace test

    memcpy(buf1, ref_buf, ref_size);
    size1 = ref_size;

    size1 = (*conv1)(buf1, size1, buf1);
    size1 = (*conv2)(buf1, size1, buf1);
    if (conv3)
      size1 = (*conv3)(buf1, size1, buf1);

    if (size1 != ref_size)
      return log->err("(inplace) output size = %i, was %i", size1, ref_size);

    if (memcmp(buf1, ref_buf, ref_size))
      return log->err("(inplace) data differs");

    // copy test
    uint8_t *check_buf;
    size_t check_size;

    memcpy(buf1, ref_buf, ref_size);
    size1 = ref_size;

    memset(buf2, 0, test_size);
    size2 = (*conv1)(buf1, size1, buf2);

    memset(buf1, 0, test_size);
    size1 = (*conv2)(buf2, size2, buf1);

    if (conv3)
    {
      memset(buf2, 0, test_size);
      size2 = (*conv3)(buf1, size1, buf2);
      check_buf = buf2;
      check_size = size2;
    }
    else
    {
      check_buf = buf1;
      check_size = size1;
    }

    if (check_size != ref_size)
      return log->err("(copy) output size = %i, was %i", check_size, ref_size);

    if (memcmp(check_buf, ref_buf, ref_size))
      return log->err("(copy) data differs");

    return log->get_errors();
  }

  void speed(bs_conv_t conv, const char *desc)
  {
    // Get noise block
    // Note that we must correct data size because
    // it may be increased during conversion.

    prepare(noise_size, 4356546);
    ref_size /= 8;
    ref_size *= 7;

    CPUMeter cpu;
    cpu.reset();
    cpu.start();

    int runs = 0;
    while (cpu.get_thread_time() < time_per_test)
    {
      runs++;
      (*conv)(ref_buf, ref_size, ref_buf);
    }
    cpu.stop();

    log->msg("Conversion %s speed: %iMB/s", 
      desc, int(ref_size * runs / cpu.get_thread_time() / 1000000));  
  }

  inline const char *bs_name(int bs_type)
  {
    switch (bs_type)
    {
      case BITSTREAM_8:    return "byte";
      case BITSTREAM_16BE: return "16be";
      case BITSTREAM_16LE: return "16le";
      case BITSTREAM_32BE: return "32be";
      case BITSTREAM_32LE: return "32le";
      case BITSTREAM_14BE: return "14be";
      case BITSTREAM_14LE: return "14le";
      default: return "??";
    }
  }

  inline bool is_14bit(int bs_type)
  {
    return bs_type == BITSTREAM_14BE || bs_type == BITSTREAM_14LE;
  }
};


///////////////////////////////////////////////////////////////////////////////
// Test function

int test_bs_convert(Log *log)
{
  BSConvert_test test(log);
  return test.test();
}
