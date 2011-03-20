/*
  AC3 test

  * IMDCT 512/256 correctness test (compare with reference algorithm)
  * MDCT 512 correctness test (compare with reference algorithm)
  * Decode encoder's output and compare internal data of ac3 encoder and decoder:
    * compare exponents
    * compare mantissas
  * IMDCT speed test
  * MDCT speed test
*/


#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "log.h"
#include "filter_tester.h"

#include "parsers\ac3\ac3_parser.h"
#include "parsers\ac3\ac3_enc.h"
#include "filters\convert.h"
#include "filter_graph.h"
#include "auto_file.h"

#include "rng.h"
#include "data.h"
#include "win32\cpu.h"


///////////////////////////////////////////////////////////////////////////////
// Test constants

static const vtime_t time_per_test = 1.0; // 1s per speed test

const sample_t ac3_window[256] = 
{
  0.00014, 0.00024, 0.00037, 0.00051, 0.00067, 0.00086, 0.00107, 0.00130,
  0.00157, 0.00187, 0.00220, 0.00256, 0.00297, 0.00341, 0.00390, 0.00443,
  0.00501, 0.00564, 0.00632, 0.00706, 0.00785, 0.00871, 0.00962, 0.01061,
  0.01166, 0.01279, 0.01399, 0.01526, 0.01662, 0.01806, 0.01959, 0.02121,
  0.02292, 0.02472, 0.02662, 0.02863, 0.03073, 0.03294, 0.03527, 0.03770,
  0.04025, 0.04292, 0.04571, 0.04862, 0.05165, 0.05481, 0.05810, 0.06153,
  0.06508, 0.06878, 0.07261, 0.07658, 0.08069, 0.08495, 0.08935, 0.09389,
  0.09859, 0.10343, 0.10842, 0.11356, 0.11885, 0.12429, 0.12988, 0.13563,
  0.14152, 0.14757, 0.15376, 0.16011, 0.16661, 0.17325, 0.18005, 0.18699,
  0.19407, 0.20130, 0.20867, 0.21618, 0.22382, 0.23161, 0.23952, 0.24757,
  0.25574, 0.26404, 0.27246, 0.28100, 0.28965, 0.29841, 0.30729, 0.31626,
  0.32533, 0.33450, 0.34376, 0.35311, 0.36253, 0.37204, 0.38161, 0.39126,
  0.40096, 0.41072, 0.42054, 0.43040, 0.44030, 0.45023, 0.46020, 0.47019,
  0.48020, 0.49022, 0.50025, 0.51028, 0.52031, 0.53033, 0.54033, 0.55031,
  0.56026, 0.57019, 0.58007, 0.58991, 0.59970, 0.60944, 0.61912, 0.62873,
  0.63827, 0.64774, 0.65713, 0.66643, 0.67564, 0.68476, 0.69377, 0.70269,
  0.71150, 0.72019, 0.72877, 0.73723, 0.74557, 0.75378, 0.76186, 0.76981,
  0.77762, 0.78530, 0.79283, 0.80022, 0.80747, 0.81457, 0.82151, 0.82831,
  0.83496, 0.84145, 0.84779, 0.85398, 0.86001, 0.86588, 0.87160, 0.87716,
  0.88257, 0.88782, 0.89291, 0.89785, 0.90264, 0.90728, 0.91176, 0.91610,
  0.92028, 0.92432, 0.92822, 0.93197, 0.93558, 0.93906, 0.94240, 0.94560,
  0.94867, 0.95162, 0.95444, 0.95713, 0.95971, 0.96217, 0.96451, 0.96674,
  0.96887, 0.97089, 0.97281, 0.97463, 0.97635, 0.97799, 0.97953, 0.98099,
  0.98236, 0.98366, 0.98488, 0.98602, 0.98710, 0.98811, 0.98905, 0.98994,
  0.99076, 0.99153, 0.99225, 0.99291, 0.99353, 0.99411, 0.99464, 0.99513,
  0.99558, 0.99600, 0.99639, 0.99674, 0.99706, 0.99736, 0.99763, 0.99788,
  0.99811, 0.99831, 0.99850, 0.99867, 0.99882, 0.99895, 0.99908, 0.99919,
  0.99929, 0.99938, 0.99946, 0.99953, 0.99959, 0.99965, 0.99969, 0.99974,
  0.99978, 0.99981, 0.99984, 0.99986, 0.99988, 0.99990, 0.99992, 0.99993,
  0.99994, 0.99995, 0.99996, 0.99997, 0.99998, 0.99998, 0.99998, 0.99999,
  0.99999, 0.99999, 0.99999, 1.00000, 1.00000, 1.00000, 1.00000, 1.00000,
  1.00000, 1.00000, 1.00000, 1.00000, 1.00000, 1.00000, 1.00000, 1.00000
};

class Ref_IMDCT
{
protected:
  sample_t xcos1[128];
  sample_t xsin1[128];
  sample_t xcos2[64];
  sample_t xsin2[64];

public:
  Ref_IMDCT()
  {
    int k;
    const int N = 512;

    ///////////////////////////////////////////////////////
    // xsin1, xcos1,  xsin2, xcos2

    for (k = 0; k < N/4; k++)
    {
      xcos1[k] = -cos(2 * M_PI * (8 * k + 1) / (8 * N));
      xsin1[k] = -sin(2 * M_PI * (8 * k + 1) / (8 * N));
    }

    for (k = 0; k < N/8; k++)
    {
      xcos2[k] = -cos(2 * M_PI * (8 * k + 1) / (4 * N));
      xsin2[k] = -sin(2 * M_PI * (8 * k + 1) / (4 * N));
    }

  };

  void imdct_512(sample_t *data, sample_t *delay)
  {
    const int N = 512;
    int k, n;
    const sample_t *w = ac3_window;

    ///////////////////////////////////////////////////////
    // Pre-IFFT complex multiply

    sample_t Z_r[N/4];
    sample_t Z_i[N/4];
    for (k = 0; k < N/4; k++)
    {
      Z_r[k] = data[N/2 - 2*k - 1] * xcos1[k] - data[2*k] * xsin1[k];
      Z_i[k] = data[2*k] * xcos1[k] + data[N/2 - 2*k - 1] * xsin1[k];
    }

    ///////////////////////////////////////////////////////
    // Complex IFFT

    sample_t z_r[N/4];
    sample_t z_i[N/4];
    for (n = 0; n < N/4; n++)
    {
      z_r[n] = 0;
      z_i[n] = 0;
      for (k = 0; k < N/4; k++)
      {
        z_r[n] += Z_r[k] * cos(8 * M_PI * k * n / N) - Z_i[k] * sin(8 * M_PI * k * n / N);
        z_i[n] += Z_i[k] * cos(8 * M_PI * k * n / N) + Z_r[k] * sin(8 * M_PI * k * n / N);
      }
    }

    ///////////////////////////////////////////////////////
    // Post-IFFT complex multiply

    sample_t y_r[N/4];
    sample_t y_i[N/4];
    for (n = 0; n < N/4; n++)
    {
      y_r[n] = z_r[n] * xcos1[n] - z_i[n] * xsin1[n];
      y_i[n] = z_i[n] * xcos1[n] + z_r[n] * xsin1[n];
    }

    ///////////////////////////////////////////////////////
    // Windowing and de-interleaving

    sample_t x[N];
    for (n = 0; n < N/8; n++)
    {
      x[2*n] = -y_i[N/8+n] * w[2*n];
      x[2*n+1] = y_r[N/8-n-1] * w[2*n+1];
      x[N/4+2*n] = -y_r[n] * w[N/4+2*n];
      x[N/4+2*n+1] = y_i[N/4-n-1] * w[N/4+2*n+1];
      x[N/2+2*n] = -y_r[N/8+n] * w[N/2-2*n-1];
      x[N/2+2*n+1] = y_i[N/8-n-1] * w[N/2-2*n-2];
      x[3*N/4+2*n] = y_i[n] * w[N/4-2*n-1];
      x[3*N/4+2*n+1]= -y_r[N/4-n-1] * w[N/4-2*n-2];
    }

    ///////////////////////////////////////////////////////
    // Overlap and add

    for (n = 0; n < N/2; n++)
    {
      data[n] = 2 * (x[n] + delay[n]);
      delay[n] = x[N/2+n];
    }
  }

  void imdct_256(sample_t *data, sample_t *delay)
  {
    const int N = 512;
    int k, n;
    const sample_t *w = ac3_window;

    sample_t X1[N/4];
    sample_t X2[N/4];
    for (k = 0; k < N/4; k++)
    {
      X1[k] = data[2*k];
      X2[k] = data[2*k+1];
    }

    ///////////////////////////////////////////////////////
    // Pre-IFFT complex multiply

    sample_t Z1_r[N/8];
    sample_t Z1_i[N/8];
    sample_t Z2_r[N/8];
    sample_t Z2_i[N/8];
    for (k = 0; k < N/8; k++)
    {
      Z1_r[k] = X1[N/4-2*k-1] * xcos2[k] - X1[2*k] * xsin2[k];
      Z1_i[k] = X1[2*k] * xcos2[k] + X1[N/4-2*k-1] * xsin2[k];
      Z2_r[k] = X2[N/4-2*k-1] * xcos2[k] - X2[2*k] * xsin2[k];
      Z2_i[k] = X2[2*k] * xcos2[k] + X2[N/4-2*k-1] * xsin2[k];
    }

    ///////////////////////////////////////////////////////
    // Complex IFFT

    sample_t z1_r[N/8];
    sample_t z1_i[N/8];
    sample_t z2_r[N/8];
    sample_t z2_i[N/8];
    for (n = 0; n < N/8; n++)
    {
      z1_r[n] = 0;
      z1_i[n] = 0;
      z2_r[n] = 0;
      z2_i[n] = 0;
      for (k = 0; k < N/8; k++)
      {
        z1_r[n] += Z1_r[k] * cos(16 * M_PI * k * n / N) - Z1_i[k] * sin(16 * M_PI * k * n / N);
        z1_i[n] += Z1_i[k] * cos(16 * M_PI * k * n / N) + Z1_r[k] * sin(16 * M_PI * k * n / N);
        z2_r[n] += Z2_r[k] * cos(16 * M_PI * k * n / N) - Z2_i[k] * sin(16 * M_PI * k * n / N);
        z2_i[n] += Z2_i[k] * cos(16 * M_PI * k * n / N) + Z2_r[k] * sin(16 * M_PI * k * n / N);
      }
    }

    ///////////////////////////////////////////////////////
    // Post-IFFT complex multiply

    sample_t y1_r[N/8];
    sample_t y1_i[N/8];
    sample_t y2_r[N/8];
    sample_t y2_i[N/8];
    for (n = 0; n < N/8; n++)
    {
      y1_r[n] = z1_r[n] * xcos2[n] - z1_i[n] * xsin2[n];
      y1_i[n] = z1_i[n] * xcos2[n] + z1_r[n] * xsin2[n];
      y2_r[n] = z2_r[n] * xcos2[n] - z2_i[n] * xsin2[n];
      y2_i[n] = z2_i[n] * xcos2[n] + z2_r[n] * xsin2[n];
    }

    ///////////////////////////////////////////////////////
    // Windowing and de-interleaving

    sample_t x[N];
    for(n = 0; n < N/8; n++)
    {
      x[2*n] = -y1_i[n] * w[2*n];
      x[2*n+1] = y1_r[N/8-n-1] * w[2*n+1];
      x[N/4+2*n] = -y1_r[n] * w[N/4+2*n];
      x[N/4+2*n+1] = y1_i[N/8-n-1] * w[N/4+2*n+1];
      x[N/2+2*n] = -y2_r[n] * w[N/2-2*n-1];
      x[N/2+2*n+1] = y2_i[N/8-n-1] * w[N/2-2*n-2];
      x[3*N/4+2*n] = y2_i[n] * w[N/4-2*n-1];
      x[3*N/4+2*n+1] = -y2_r[N/8-n-1] * w[N/4-2*n-2];
    }

    ///////////////////////////////////////////////////////
    // Overlap and add

    for (n = 0; n < N/2; n++)
    {
      data[n] = 2 * (x[n] + delay[n]);
      delay[n] = x[N/2+n];
    }
  }
};


class Ref_MDCT
{
public:
  Ref_MDCT() {};

  void mdct512(sample_t data[512], sample_t output[256])
  {
    const int N = 512;
    for (int k = 0; k < N/2; k++)
    {
      output[k] = 0;
      for (int n = 0; n < N; n++)
        output[k] += data[n] * cos( (2*M_PI) / (4*N) * (2*n+1) * (2*k+1) + M_PI/4*(2*k+1) );
      output[k] *= -2.0/N;
    }
  }
};



class AC3Test
{
protected:
  Log *log;

public:
  AC3Test(Log *_log)
  {
    log = _log;
  }

  int test()
  {
    int err = 0;

    log->open_group("AC3 encode-decode test");
    test_ac3_encdec("a.pcm.005.lpcm", Speakers(FORMAT_PCM16_BE, MODE_STEREO, 48000), 640000, 375);
    err += log->close_group();

    log->open_group("DCT tests");
    test_imdct512();
    test_imdct256();
    test_mdct512();
    speed_imdct();
    speed_mdct();

    err += log->close_group();

    return err;
  }

  sample_t max_diff(sample_t init, sample_t *x, sample_t *y, int n)
  {
    sample_t diff = init;
    for (int i = 0; i < n; i++)
      if (fabs(x[i] - y[i]) > diff)
        diff = fabs(x[i] - y[i]);
    return diff;
  }

  int test_imdct512()
  {
    int i;
    const int blocks = 10;

    sample_t data[AC3_BLOCK_SAMPLES * blocks];
    sample_t data_ref[AC3_BLOCK_SAMPLES * blocks];
    sample_t delay[AC3_BLOCK_SAMPLES];
    sample_t delay_ref[AC3_BLOCK_SAMPLES];

    ///////////////////////////////////////////////////////
    // init input

    RNG rng(234256);
    for (i = 0; i < AC3_BLOCK_SAMPLES * blocks; i++)
    {
      data[i] = rng.get_sample();
      data_ref[i] = data[i];
    }

    for (i = 0; i < AC3_BLOCK_SAMPLES; i++)
    {
      delay[i] = 0;
      delay_ref[i] = 0;
    }

    ///////////////////////////////////////////////////////
    // Do imdct and compare result

    IMDCT imdct;
    Ref_IMDCT imdct_ref;
    sample_t diff = 0;

    for (i = 0; i < blocks; i++)
    {
      imdct.imdct_512(data + AC3_BLOCK_SAMPLES * i, delay);
      imdct_ref.imdct_512(data_ref + AC3_BLOCK_SAMPLES * i, delay_ref);
      diff = max_diff(diff, data + AC3_BLOCK_SAMPLES * i, data_ref + AC3_BLOCK_SAMPLES * i, AC3_BLOCK_SAMPLES);
      if (diff > 1e-8) return log->err("IMDCT 512 is wrong");
    }

    log->msg("IMDCT 512: max difference = %.0e", diff);
    return 0;
  }

  int test_imdct256()
  {
    int i;
    const int blocks = 10;

    sample_t data[AC3_BLOCK_SAMPLES * blocks];
    sample_t data_ref[AC3_BLOCK_SAMPLES * blocks];
    sample_t delay[AC3_BLOCK_SAMPLES];
    sample_t delay_ref[AC3_BLOCK_SAMPLES];

    ///////////////////////////////////////////////////////
    // init input

    RNG rng(234256);
    for (i = 0; i < AC3_BLOCK_SAMPLES * blocks; i++)
    {
      data[i] = rng.get_sample();
      data_ref[i] = data[i];
    }

    for (i = 0; i < AC3_BLOCK_SAMPLES; i++)
    {
      delay[i] = 0;
      delay_ref[i] = 0;
    }

    ///////////////////////////////////////////////////////
    // Do imdct and compare result

    IMDCT imdct;
    Ref_IMDCT imdct_ref;
    sample_t diff = 0;

    for (i = 0; i < blocks; i++)
    {
      imdct.imdct_256(data + AC3_BLOCK_SAMPLES * i, delay);
      imdct_ref.imdct_256(data_ref + AC3_BLOCK_SAMPLES * i, delay_ref);
      diff = max_diff(diff, data + AC3_BLOCK_SAMPLES * i, data_ref + AC3_BLOCK_SAMPLES * i, AC3_BLOCK_SAMPLES);
      if (diff > 1e-8) return log->err("IMDCT 256 is wrong");
    }

    log->msg("IMDCT 256: max difference = %.0e", diff);
    return 0;
  }

  void speed_imdct()
  {
    IMDCT imdct;
    SampleBuf samples;
    SampleBuf delay;
    RNG rng(234256);

    samples.allocate(1, AC3_BLOCK_SAMPLES);
    delay.allocate(1, AC3_BLOCK_SAMPLES);
    rng.fill_samples(samples[0], AC3_BLOCK_SAMPLES);
    rng.fill_samples(delay[0], AC3_BLOCK_SAMPLES);

    CPUMeter cpu;
    cpu.reset();
    cpu.start();

    int blocks = 0;
    while (cpu.get_thread_time() < time_per_test)
    {
      imdct.imdct_512(samples[0], delay[0]);
      blocks++;
    }
    cpu.stop();
    
    log->msg("IMDCT blocks/s: %i", int(blocks / cpu.get_thread_time()));
  }

  sample_t max_diff(sample_t init, int32_t *x, sample_t *y, int n)
  {
    sample_t diff = init;
    for (int i = 0; i < n; i++)
      if (fabs(sample_t(x[i]) - y[i] * 32768.0) > diff)
        diff = fabs(sample_t(x[i]) - y[i] * 32768.0);
    return diff;
  }

  int test_mdct512()
  {
    int i;
    const int blocks = 10;
    const sample_t *w = ac3_window;

    int16_t  data[512 * blocks];
    int32_t  output[256];
    sample_t data_ref[512 * blocks];
    sample_t output_ref[256];

    ///////////////////////////////////////////////////////
    // init input

    RNG rng(234256);
    for (int block = 0; block < blocks; block++)
    {
      for (i = 0; i < 256; i++)
      {
        data_ref[512 * block + i] = rng.get_sample() * w[i];
        data_ref[512 * block + i + 256] = rng.get_sample() * w[255 - i];
        data[512 * block + i] = int16_t(data_ref[512 * block + i] * 32767);
        data[512 * block + i + 256] = int16_t(data_ref[512 * block + i + 256] * 32767);
      }
    }

    ///////////////////////////////////////////////////////
    // Do imdct and compare result

    MDCT mdct(7);
    Ref_MDCT mdct_ref;
    sample_t diff = 0;

    for (i = 0; i < blocks; i++)
    {
      mdct.mdct512(output, data + 512 * i);
      mdct_ref.mdct512(data_ref + 512 * i, output_ref);
      diff = max_diff(diff, output, output_ref, 256);
      if (diff > 8) return log->err("MDCT is wrong");
    }

    log->msg("MDCT 512: max difference = %i", int(diff));
    return 0;
  }

  void speed_mdct()
  {
    int s;
    int16_t mdct_buf[AC3_BLOCK_SAMPLES * 2];
    int16_t delay[AC3_BLOCK_SAMPLES];
    int32_t mant[AC3_BLOCK_SAMPLES];
    SampleBuf input;
    MDCT mdct(7);
    RNG rng;

    input.allocate(1, AC3_BLOCK_SAMPLES);

    rng.fill_samples(input[0], AC3_BLOCK_SAMPLES);
    rng.fill_raw(delay, AC3_BLOCK_SAMPLES * 2);

    CPUMeter cpu;
    cpu.reset();
    cpu.start();

    int blocks = 0;
    while (cpu.get_thread_time() < time_per_test)
    {
      sample_t v;
      sample_t *sptr = input[0];
      memcpy(mdct_buf, delay, sizeof(delay));
      for (s = 0; s < AC3_BLOCK_SAMPLES; s++)
      {
        v = *sptr++;
        mdct_buf[s + 256] = int32_t(v * ac3_window[AC3_BLOCK_SAMPLES - s - 1]);
        delay[s] = int32_t(v * ac3_window[s]);
      }
      mdct.mdct512(mant, mdct_buf);
      blocks++;
    }
    cpu.stop();
    
    log->msg("MDCT blocks/s: %i", int(blocks / cpu.get_thread_time()));

  }

  int test_ac3_encdec(const char *_raw_filename, Speakers _spk, int _bitrate, int _nframes)
  {
    log->msg("Testing file %s...", _raw_filename);

    AutoFile f(_raw_filename);
    if (!f.is_open())
      return log->err("Cannot open file '%s'", _raw_filename);

    const int buf_size = 32000;
    uint8_t buf[buf_size];
    int buf_data;

    int frames = 0;
    uint8_t *frame_pos;
    Speakers raw_spk = _spk;
    Speakers lin_spk = _spk;
    lin_spk.format = FORMAT_LINEAR;

    Converter conv(2048);
    AC3Enc    enc;
    AC3Parser dec;

    FilterChain chain;
    chain.add_back(&conv, "Converter");
    chain.add_back(&enc,  "Encoder");

    conv.set_buffer(AC3_FRAME_SAMPLES);
    conv.set_format(FORMAT_LINEAR);
    conv.set_order(win_order);

    if (!enc.set_bitrate(_bitrate) ||
        !enc.set_input(lin_spk))
      return log->err("Cannot init encoder!");

    dec.do_imdct = false;
    dec.do_dither = false;

    Chunk raw;
    Chunk ac3;
  
    while (!f.eof())
    {
      buf_data = f.read(buf, buf_size);

      raw.set_rawdata(raw_spk, buf, buf_data);

      if (!chain.process(&raw))
        return log->err("error in chain.process()");

      while (!chain.is_empty())
      {
        if (!chain.get_chunk(&ac3))
          return log->err("error in chain.get_chunk()");

        if (ac3.is_empty())
          continue;

        frame_pos = ac3.rawdata;
        if (!dec.start_parse(frame_pos, ac3.size))
          return log->err("AC3 start parsing error!");

        if (!dec.parse_header())
          return log->err("AC3 header parsing error!");

        for (int b = 0; b < AC3_NBLOCKS; b++)
        {
          if (!dec.decode_block())
            return log->err("block %i decode error!", b);

          if (memcmp(enc.exp[0][b], dec.exps[0], 223) || 
              memcmp(enc.exp[1][b], dec.exps[1], 223))
            return log->err("exponents error!", b);

          for (int ch = 0; ch < _spk.nch(); ch++)
          {
            int endmant;
            if (_spk.lfe() && ch != _spk.nch())
              // lfe channel
              endmant = 7;
            else
              // fbw channel
              if (enc.nmant[ch] != dec.endmant[ch])
                return log->err("number of mantissas does not match!", b);
              else
                endmant = enc.nmant[ch];

            for (int s = 0; s < endmant; s++)
            {
              // quantize/dequantize encoded dct coefs and compare with decoded coefs
              double v = 0;
              double v1 = 0;
              int bap = enc.bap[ch][b][s];
              int m = enc.mant[ch][b][s];
              int e = enc.exp[ch][b][s];

              switch (bap)
              {
                case 0:  break;
                // asymmetric quantization
                case 1:  v = sym_quant(m, 3)  * 2.0/3.0  - 2.0/3.0;   break;
                case 2:  v = sym_quant(m, 5)  * 2.0/5.0  - 4.0/5.0;   break;
                case 3:  v = sym_quant(m, 7)  * 2.0/7.0  - 6.0/7.0;   break;
                case 4:  v = sym_quant(m, 11) * 2.0/11.0 - 10.0/11.0; break;
                case 5:  v = sym_quant(m, 15) * 2.0/15.0 - 14.0/15.0; break;
                // symmetric quantization
                case 6:  v = int16_t(asym_quant(m, 5)  << 11) / 32768.0; break;
                case 7:  v = int16_t(asym_quant(m, 6)  << 10) / 32768.0; break;
                case 8:  v = int16_t(asym_quant(m, 7)  << 9)  / 32768.0; break;
                case 9:  v = int16_t(asym_quant(m, 8)  << 8)  / 32768.0; break;
                case 10: v = int16_t(asym_quant(m, 9)  << 7)  / 32768.0; break;
                case 11: v = int16_t(asym_quant(m, 10) << 6)  / 32768.0; break;
                case 12: v = int16_t(asym_quant(m, 11) << 5)  / 32768.0; break;
                case 13: v = int16_t(asym_quant(m, 12) << 4)  / 32768.0; break;
                case 14: v = int16_t(asym_quant(m, 14) << 2)  / 32768.0; break;
                case 15: v = int16_t(asym_quant(m, 16))       / 32768.0; break;
              }

              v1 = dec.get_samples()[ch][s + b * AC3_BLOCK_SAMPLES] * (1 << e);
              if (fabs(v - v1) > 1e-10)
                log->msg("strange sample f=%i ch=%i b=%i s=%i; bap=%i, mant=%i, exp=%i, v=%e, s=%e, v-s=%e...", frames, ch, b, s, bap, m, e, double(sym_quant(m, 11)) * 2.0/11.0 - 10.0/11.0, dec.get_samples()[ch][s + b * AC3_BLOCK_SAMPLES], fabs(v - v1));
            }
          } // for (int ch = 0; ch < _spk.nch(); ch++)
        } // for (int b = 0; b < AC3_NBLOCKS; b++)

        frames++;
        log->status("Frame %i", frames);
      }
    }

    if (_nframes && _nframes != frames)
      return log->err("number of encoded frames (%i) does not match correct number (%i)!", frames, _nframes);

    return 0;
  }

};





///////////////////////////////////////////////////////////////////////////////
// Test function

int test_ac3(Log *log)
{
  AC3Test test(log);
  return test.test();
}
