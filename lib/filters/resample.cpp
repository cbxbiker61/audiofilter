#include <math.h>
#include <string.h>
#include "resample.h"
#include "../dsp/kaiser.h"
#include "../dsp/fftsg.h"

static const double k_conv = 2;
static const double k_fft = 20.1977305724455;

///////////////////////////////////////////////////////////////////////////////
// Math

inline double sinc(double x) { return x == 0 ? 1 : sin(x)/x; }
inline double lpf(int i, double freq) { return 2 * freq * sinc(i * 2 * M_PI * freq); }
inline int gcd(int x, int y);
inline unsigned int flp2(unsigned int x);
inline unsigned int clp2(unsigned int x);

double t_upsample(int l1, int m1, int l2, int m2, double a, double q);
double t_downsample(int l1, int m1, int l2, int m2, double a, double q);
double optimize_upsample(int l, int m, double a, double q, int &l1, int &m1, int &l2, int &m2);
double optimize_downsample(int l, int m, double a, double q, int &l1, int &m1, int &l2, int &m2);

///////////////////////////////////////////////////////////////////////////////
// Resample class definition
///////////////////////////////////////////////////////////////////////////////

Resample::Resample(): 
  NullFilter(FORMAT_MASK_LINEAR),
  a(100.0), q(0.99), fs(0), fd(0), nch(0), rate(1.0),
  g(0), l(0), m(0), l1(0), l2(0), m1(0), m2(0),
  n1(0), n1x(0), n1y(0),
  c1(0), c1x(0), c1y(0),
  f1_raw(0), f1(0), order(0),
  n2(0), n2b(0), c2(0), f2(0),
  fft_ip(0), fft_w(0)
{
  for (int i = 0; i < NCHANNELS; i++)
  {
    buf1[i] = 0;
    buf2[i] = 0;
    delay2[i] = 0;
  }

  sample_rate = 0;
  out_spk.set_unknown();
  out_samples.zero();
  out_size = 0;
};

Resample::Resample(int _sample_rate, double _a, double _q): 
  NullFilter(FORMAT_MASK_LINEAR),
  a(100.0), q(0.99), fs(0), fd(0), nch(0), rate(1.0),
  g(0), l(0), m(0), l1(0), l2(0), m1(0), m2(0),
  n1(0), n1x(0), n1y(0),
  c1(0), c1x(0), c1y(0),
  f1_raw(0), f1(0), order(0),
  n2(0), n2b(0), c2(0), f2(0),
  fft_ip(0), fft_w(0)
{
  for (int i = 0; i < NCHANNELS; i++)
  {
    buf1[i] = 0;
    buf2[i] = 0;
    delay2[i] = 0;
  }

  sample_rate = 0;
  out_spk.set_unknown();
  out_samples.zero();
  out_size = 0;

  set(_sample_rate, _a, _q);
};

Resample::~Resample()
{
  uninit_resample();
}



///////////////////////////////////////////////////////////////////////////////
// User interface
///////////////////////////////////////////////////////////////////////////////

bool
Resample::set(int _sample_rate, double _a, double _q)
{
  if (_sample_rate < 0) return false;
  if (_a < 6) return false;
  if (_q < 0.1) return false;
  if (_q >= 0.9999999999) return false;
  
  sample_rate = _sample_rate;
  a = _a;
  q = _q;

  uninit_resample();
  out_spk = spk;
  if (!spk.is_unknown() && sample_rate != 0)
  {
    out_spk.sample_rate = sample_rate;
    if (spk.sample_rate != sample_rate)
      init_resample(spk.nch(), spk.sample_rate, sample_rate);
  }

  return true;
}

void
Resample::get(int *_sample_rate, double *_a, double *_q)
{
  if (_sample_rate) *_sample_rate = sample_rate;
  if (_a) *_a = a;
  if (_q) *_q = q;
}

///////////////////////////////////////////////////////////////////////////////
// Filter interface
///////////////////////////////////////////////////////////////////////////////

void
Resample::reset()
{
  NullFilter::reset();
  if (sample_rate)
    reset_resample();
}

bool
Resample::set_input(Speakers _spk)
{
  FILTER_SAFE(NullFilter::set_input(_spk));

  uninit_resample();
  out_spk = _spk;

  if (sample_rate)
  {
    out_spk.sample_rate = sample_rate;
    if (spk.sample_rate != sample_rate)
      init_resample(spk.nch(), spk.sample_rate, sample_rate);
  }
  return true;
}

Speakers
Resample::get_output() const
{
  return out_spk;
}

bool
Resample::get_chunk(Chunk *_chunk)
{
  if (sample_rate && spk.sample_rate != sample_rate)
  {
    if (size)
    {
      size_t processed = process_resample(samples.samples, size);
      drop_samples(processed);
      _chunk->set_linear(out_spk, out_samples, out_size);
    }
    else if (flushing)
    {
      bool send_eos = flush_resample();
      _chunk->set_linear(out_spk, out_samples, out_size);
      if (send_eos)
      {
        _chunk->set_eos();
        reset();
      }
    }

    if (sync)
    {
      _chunk->set_sync(true, time);
      sync = false;
    }

    return true;
  }

  send_chunk_inplace(_chunk, size);
  return true;
}


///////////////////////////////////////////////////////////////////////////////
// Init
///////////////////////////////////////////////////////////////////////////////

int
Resample::init_resample(int _nch, int _fs, int _fd)
{
  int i;
  uninit_resample();

  fs = _fs; // source sample rate
  fd = _fd; // destinationsample rate
  nch = _nch; // number fo channels
  rate = double(fd) / double(fs);

  g = gcd(fs, fd);
  l = fd / g; // interpolation factor
  m = fs / g; // decimation factor

  if (fs < fd)
    optimize_upsample(l, m, a, q, l1, m1, l2, m2);
  else
    optimize_downsample(l, m, a, q, l1, m1, l2, m2);

  ///////////////////////////////////////////////////////////////////////////
  // We can consider the attenuation as amount of noise introduced by a filter.
  // Two stages introduces twice more noise, so to keep the output noise below
  // the user-specified, we should add 6dB attenuation to both stages.
  // Also, the noise in the stopband, produced at each stage is folded into
  // the passband during decimation. Decimation factor is the noise gain level,
  // so we should add it to the attenuation.

  double alpha;                     // alpha parameter for the kaiser window
  double a1 = a + log10(double(m1))*20 + 6; // convolution stage attenuation
  double a2 = a + log10(double(m2))*20 + 6; // fft stage attenuation

  ///////////////////////////////////////////////////////////////////////////
  // Find filters' parameters: transition band width and cennter frequency

  double phi = double(l1) / double(m1);
  double df1, lpf1, df2, lpf2;

  if (fs < fd) // upsample
  {
    // convolution stage
    df1  = (phi - q) / (2 * l1);
    lpf1 = (phi + q) / (4 * l1);
    // fft stage
    df2  = (1 - q) / (2 * phi * l2);
    lpf2 = (1 + q) / (4 * phi * l2);
  }
  else // downsample
  {
    // convolution stage
    df1  = (phi - q * rate) / (2 * l1);
    lpf1 = (phi + q * rate) / (4 * l1);
    // fft stage
    df2  = rate * (1 - q) / (2 * phi * l2);
    lpf2 = rate * (1 + q) / (4 * phi * l2);
  }

  ///////////////////////////////////////////////////////////////////////////
  // Build convolution stage filter

  // find the fiter length
  n1 = kaiser_n(a1, df1) | 1;
  n1x = (n1 + l1 - 1) / l1;     // make n1x odd; larger, because we
  n1x = n1x | 1;     // should not make the filter weaker
  n1y = l1;
  n1 = n1x * n1y;    // use all available space for the filter
  n1 = n1 - 1 | 1;   // make n1 odd (type1 filter); smaller, because we
                     // must fit the filter into the given space
  c1 = (n1 - 1) / 2; // center of the filter

  // allocate the filter
  // f1[n1y][n1x]
  f1 = new sample_t *[n1y];
  f1[0] = new sample_t[n1x * n1y];
  for (i = 0; i < n1x * n1y; i++) f1[0][i] = 0;
  for (i = 1; i < n1y; i++) f1[i] = f1[0] + i * n1x;

  f1_raw = new sample_t[n1y * n1x];
  for (i = 0; i < n1x * n1y; i++) f1_raw[i] = 0;

  // build the filter
  alpha = kaiser_alpha(a1);
  for (i = 0; i < n1; i++)
    f1_raw[i] = (sample_t) (kaiser_window(i - c1, n1, alpha) * lpf(i - c1, lpf1) * l1);

  // reorder the filter
  // find coordinates of the filter's center
  for (int y = 0; y < n1y; y++)
    for (int x = 0; x < n1x; x++)
    {
      int p = l1-1 - (y*m1)%l1 + x*l1;
      f1[y][x] = f1_raw[p];
      if (p == c1)
        c1x = x, c1y = y;
    }

  // data ordering
  order = new int[l1];
  for (i = 0; i < l1; i++) 
    order[i] = i * m1 / l1;

  ///////////////////////////////////////////////////////////////////////////
  // Build fft stage filter

  // filter length must be odd (type 1 filter), but fft length must be even;
  // therefore n2 is even, but only n2-1 bins will be used for the filter
  n2 = kaiser_n(a2, df2) | 1;
  n2 = clp2(n2);
  n2b = n2*2;
  c2 = n2 / 2 - 1;

  // allocate the filter
  // f2[n2b]
  f2 = new sample_t[n2b];
  for (i = 0; i < n2b; i++) f2[i] = 0;

  // make the filter
  // filter length is n2-1
  alpha = kaiser_alpha(a2);
  for (i = 0; i < n2-1; i++)
    f2[i] = (sample_t)(kaiser_window(i - c2, n2-1, alpha) * lpf(i - c2, lpf2) * l2 / n2);

  // convert the filter to frequency domain and init fft for future use
  fft_ip    = new int[(int)(2 + sqrt(double(n2b)))];
  fft_w     = new sample_t[n2b/2];
  fft_ip[0] = 0;

  rdft(n2b, 1, f2, fft_ip, fft_w);

  ///////////////////////////////////////////////////////
  // Allocate buffers

  const size_t buf1_size = n2*m1/l1+n1x+1;
  buf1[0] = new sample_t[buf1_size * nch];
  for (i = 1; i < nch; i++)
    buf1[i] = buf1[0] + i * buf1_size;

  buf2[0] = new sample_t[n2b * nch];
  for (i = 1; i < nch; i++)
    buf2[i] = buf2[0] + i * n2b;

  const size_t delay2_size = n2/m2+1;
  delay2[0] = new sample_t[delay2_size * nch];
  for (i = 1; i < nch; i++)
    delay2[i] = delay2[0] + i * delay2_size;

  out_samples.zero();
  for (i = 0; i < nch; i++)
    out_samples[i] = buf2[i];

  out_size = 0;
  reset();

#if RESAMPLE_PERF
  stage1.reset();
  stage2.reset();
#endif

  return true;
}

void
Resample::uninit_resample()
{
  if (f1) safe_delete(f1[0]);
  safe_delete(f1);
  safe_delete(f1_raw);
  safe_delete(order);
  safe_delete(f2);
  safe_delete(fft_ip);
  safe_delete(fft_w);

  safe_delete(buf1[0]);
  safe_delete(buf2[0]);
  safe_delete(delay2[0]);

  fs = 0; fd = 0; nch = 0; rate = 1.0;
  g = 0; l = 0; m = 0; l1 = 0; l2 = 0; m1 = 0; m2 = 0;
  n1 = 0; n1x = 0; n1y = 0;
  c1 = 0; c1x = 0; c1y = 0;
  f1_raw = 0; f1 = 0; order = 0;
  n2 = 0; n2b = 0; c2 = 0; f2 = 0;
  fft_ip = 0; fft_w = 0;

  out_samples.zero();
  for (int i = 0; i < NCHANNELS; i++)
  {
    buf1[i] = 0;
    buf2[i] = 0;
    delay2[i] = 0;
  }
}



///////////////////////////////////////////////////////////////////////////////
// Processing functions
///////////////////////////////////////////////////////////////////////////////

inline void
Resample::do_stage1(sample_t *in[], sample_t *out[], int n_in, int n_out)
{
  assert(n_in == stage1_in(n_out) || n_out == stage1_out(n_in));

#if RESAMPLE_PERF
  stage1.start();
#endif

  for (int ch = 0; ch < nch; ch++)
  {
    int i = pos_l;
    int n = n_out;
    sample_t *iptr = in[ch] - pos_m;
    sample_t *optr = out[ch] - pos_l;

    // Now iptr points to the 'imaginary' beginning of the block of M input
    // samples and optr points to the beginning of the block of L output
    // samples, so pos_m and pos_l are indexes at these blocks.
    //
    // But here a special case is possible. Consider L=3, M=5 and pos_m=4
    // (4 input samples processed and 3 output samples generated). In this
    // case pos_l=0 and order[pos_l]=0. So in[ch]-pos_m+order[pos_l] < in[ch].
    // Thus we must skip last (unused) samples of the input block.

    if (order[pos_l] < pos_m)
       iptr += m1;

    while (n--)
    {
      double sum = 0;
      for (int j = 0; j < n1x; j++)
        sum += iptr[order[i] + j] * f1[i][j];
      optr[i] = sum;

      i++;
      if (i >= l1)
      {
        i = 0;
        iptr += m1;
        optr += l1;
      }
    }
  }
  pos_m = (pos_m + n_in) % m1;
  pos_l = (pos_l + n_out) % l1;

#if RESAMPLE_PERF
  stage1.stop();
#endif
}

inline void
Resample::do_stage2()
{
#if RESAMPLE_PERF
  stage2.start();
#endif

  for (int ch = 0; ch < nch; ch++)
  {
    memset(buf2[ch] + n2, 0, n2 * sizeof(sample_t));

    rdft(n2b, 1, buf2[ch], fft_ip, fft_w);

    buf2[ch][0] = f2[0] * buf2[ch][0];
    buf2[ch][1] = f2[1] * buf2[ch][1]; 

    for (int i = 1; i < n2; i++)
    {
      sample_t re,im;
      re = f2[i*2  ] * buf2[ch][i*2] - f2[i*2+1] * buf2[ch][i*2+1];
      im = f2[i*2+1] * buf2[ch][i*2] + f2[i*2  ] * buf2[ch][i*2+1];
      buf2[ch][i*2  ] = re;
      buf2[ch][i*2+1] = im;
    }

    rdft(n2b, -1, buf2[ch], fft_ip, fft_w);
  }

#if RESAMPLE_PERF
  stage2.stop();
#endif
}

void
Resample::drop_pre_samples()
{
  if (pre_samples > out_size)
  {
    pre_samples -= out_size;
    out_size = 0;
  }
  else
  {
    out_size -= pre_samples;
    for (int ch = 0; ch < nch; ch++)
      memmove(out_samples[ch], out_samples[ch] + pre_samples, out_size * sizeof(sample_t));
    pre_samples = 0;
  }
}

void
Resample::reset_resample()
{
  int ch;
  if (fs && fd)
  {
    pos_l = c1y;
    pos_m = pos_l * m1 / l1;

    pre_samples = c2 / m2;
    post_samples = c1x;

    // To avoid signal shift we add c1x zero samples to the beginning,
    // so the first sample processed is guaranteed to match the center
    // of the filter.
    // Also, we should choose 'shift' value in such way, so
    // shift + pre_samples*m2 = c2

    pos1 = c1x;
    shift = c2 - pre_samples*m2;

    for (ch = 0; ch < nch; ch++)
      memset(buf1[ch], 0, pos1 * sizeof(sample_t));

    for (ch = 0; ch < nch; ch++)
      memset(delay2[ch], 0, (n2/m2+1) * sizeof(sample_t));
  }
}

size_t
Resample::process_resample(sample_t *in_buf[], size_t nsamples)
{
  int ch, i, j;
  size_t processed = 0;
  out_size = 0;

  ///////////////////////////////////////////////////////
  // Sync

  if (sync)
  {
    time -= double(pos1 - c1x) / double(fs);
    time -= double(c2) / double(fd * m2);
    time += double(pre_samples) / double(fd);
  }

  ///////////////////////////////////////////////////////
  // Stage 1 processing

  int n = n2*m1/l1 + n1x + 1;
  if (nsamples < (size_t)n - pos1)
  {
    for (ch = 0; ch < nch; ch++)
      memcpy(buf1[ch] + pos1, in_buf[ch], nsamples * sizeof(sample_t));
    pos1 += (int)nsamples;
    return nsamples;
  }
  for (ch = 0; ch < nch; ch++)
    memcpy(buf1[ch] + pos1, in_buf[ch], (n - pos1) * sizeof(sample_t));
  processed = n - pos1;

  int n_out = n2;
  int n_in = stage1_in(n2);
  do_stage1(buf1, buf2, n_in, n_out);

  pos1 = n - n_in;
  for (ch = 0; ch < nch; ch++)
    memmove(buf1[ch], buf1[ch] + n_in, pos1 * sizeof(sample_t));

  ///////////////////////////////////////////////////////
  // Stage 2 processing

  do_stage2();

  // Decimate and overlap
  // The size of output data is always less or equal to
  // the size of input data. Therefore we can process it
  // in-place.

  i = shift; j = 0;
  for (ch = 0; ch < nch; ch++)
    for (i = shift, j = 0; i < n2; i += m2, j++)
      buf2[ch][j] = buf2[ch][i] + delay2[ch][j];
  shift = i - n2;
  out_size = j;

  for (ch = 0; ch < nch; ch++)
    for (i = n2 + shift, j = 0; i < n2b; i += m2, j++)
      delay2[ch][j] = buf2[ch][i];

  ///////////////////////////////////////////////////////
  // Drop null samples from the beginning

  if (pre_samples > 0)
    drop_pre_samples();

  return processed;
}

bool
Resample::flush_resample()
{
  int ch;

  out_size = 0;
  int n = n2*m1/l1 + n1x + 1 - pos1;
  int actual_out_size = (stage1_out(pos1 - c1x) + c2 - shift) / m2 - pre_samples;
  if (!actual_out_size)
    return true;

  for (ch = 0; ch < nch; ch++)
    memset(buf1[ch] + pos1, 0, n * sizeof(sample_t));
  post_samples -= n;
  pos1 += n;

  process_resample(samples.samples, 0);

  if (post_samples <= 0)
  {
    if (actual_out_size > out_size)
    {
      // If we have no enough data, then
      // copy the rest from the delay buffer
      for (ch = 0; ch < nch; ch++)
        memcpy(buf2[ch] + out_size, delay2[ch], (actual_out_size - out_size) * sizeof(sample_t));
    }
    out_size = actual_out_size;
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// Optimization functions
///////////////////////////////////////////////////////////////////////////////

double t_upsample(int l1, int m1, int l2, int m2, double a, double q)
{
  double phi = double(l1) / double(m1);
  double alpha_conv = (a + log10(double(m1))*20 + 6 - 7.95) / 14.36;
  double alpha_fft  = (a + log10(double(m2))*20 + 6 - 7.95) / 14.36;

  double t_conv = 2 * alpha_conv * k_conv / (phi - q);
  double t_fft = k_fft * phi * l2 * log(double(2 * clp2(int(2 * alpha_fft * phi * l2 / (1 - q)))));
  return t_fft + t_conv;
}

double t_downsample(int l1, int m1, int l2, int m2, double a, double q)
{
  double phi = double(l1) / double(m1);
  double rate = double(l1 * l2) / double(m1 * m2);
  double alpha_conv = (a + log10(double(m1))*20 + 6 - 7.95) / 14.36;
  double alpha_fft  = (a + log10(double(m2))*20 + 6 - 7.95) / 14.36;

  double t_conv = 2 * alpha_conv * k_conv / (phi - q * rate);
  double t_fft = k_fft * phi * l2 * log(double(2 * clp2(int(2 * alpha_fft * phi * l2 / rate / (1 - q)))));
  return t_fft + t_conv;
}

double optimize_upsample(int l, int m, double a, double q, int &l1, int &m1, int &l2, int &m2)
{
  l1 = l; m1 = m;
  l2 = 1; m2 = 1;
  double t_opt = t_upsample(l, m, 1, 1, a, q);

  for (int m2i = 2; m2i < m; m2i++)
  {
    int g = gcd(l * m2i, m);
    double t = t_upsample(l * m2i / g, m / g, 1, m2i, a, q);
    if (t < t_opt)
    {
      t_opt = t;
      l1 = l * m2i / g;
      m1 = m / g;
      l2 = 1;
      m2 = m2i;
    }
    else if (t > 10 * t_opt)
      return t_opt;
  }
  return t_opt;
}

double optimize_downsample(int l, int m, double a, double q, int &l1, int &m1, int &l2, int &m2)
{
  l1 = l; m1 = m;
  l2 = 1; m2 = 1;
  double t_opt = t_downsample(l, m, 1, 1, a, q);

  for (int m2i = 2; m2i < m; m2i++)
  {
    int g = gcd(l * m2i, m);
    double t = t_downsample(l * m2i / g, m / g, 1, m2i, a, q);
    if (t < t_opt)
    {
      t_opt = t;
      l1 = l * m2i / g;
      m1 = m / g;
      l2 = 1;
      m2 = m2i;
    }
    else if (t > 10 * t_opt)
      return t_opt;
  }
  return t_opt;
}

///////////////////////////////////////////////////////////////////////////////
// Math
///////////////////////////////////////////////////////////////////////////////

inline int flp2(int x, int y)
{
  // largest power-of-2 <= x
  x = x | (x >> 1);
  x = x | (x >> 2);
  x = x | (x >> 4);
  x = x | (x >> 8);
  x = x | (x >> 16);
  return x - (x >> 1);
}

inline unsigned int clp2(unsigned int x)
{
  // smallest power-of-2 >= x
  x = x - 1;
  x = x | (x >> 1);
  x = x | (x >> 2);
  x = x | (x >> 4);
  x = x | (x >> 8);
  x = x | (x >> 16);
  return x + 1;
}

inline int gcd(int x, int y)
{
  int t;
  while (y != 0)
  {
    t = x % y; 
    x = y; 
    y = t;
  }
  return x;
}
