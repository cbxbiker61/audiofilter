#ifndef VALIB_RESAMPLE_H
#define VALIB_RESAMPLE_H

#include "../filter.h"
#if RESAMPLE_PERF
#include "../win32\cpu.h"
#endif

class Resample : public NullFilter
{
protected:
  // user-defined
  double a;   // attenuation factor [dB]
  double q;   // quality (passband width)

  int fs;     // source sample rate [Hz]
  int fd;     // destination sample rate [Hz]
  int nch;    // number fo channels
  double rate;// conversion rate

  // useful in calculations
  int g;      // gcd(fs, fd)
  int l, m;   // l/m - interpolation/decimation factor
  int l1, l2; // stage1/stage2 interpolation factor
  int m1, m2; // stage1/stage2 decimation factor

  // convolution stage filter
  int n1, n1x, n1y; // filter length, x and y lengths
  int c1, c1x, c1y; // center of the filter, x and y coordinates
  sample_t **f1;    // reordered filter [n1y][n1x]
  sample_t *f1_raw; // raw filter [n1y * n1x]
  int *order;       // input positions [l]

  // fft stage filter
  int n2, n2b;      // filter size and fft size
  int c2;           // center of the filter
  sample_t *f2;     // filter [n2b]

  // fft
  int      *fft_ip;
  sample_t *fft_w;

  // processing
  int pos_l, pos_m;            // stage1 convolution positions [0..l1), [0..m1)
  int pos1;                    // stage1 buffer position
  sample_t *buf1[NCHANNELS];   // stage1 buffer
  sample_t *buf2[NCHANNELS];   // stage2 buffer [n2b]
  sample_t *delay2[NCHANNELS]; // fft stage delay buffer [n2/m2]
  int shift;                   // fft stage decimation shift
  int pre_samples;             // number of samples to drop from the beginning of output data
  int post_samples;            // number of samples to add to the end of input data

  int init_resample(int _nch, int _fs, int _fd);
  void uninit_resample();

  // stage1_in(): how much input samples required to generate N output samples
  // stage1_out(): how much output samples can be made out of N input samples
  // Note that stage1_out(stage1_in(N)) >= N
  inline int stage1_in(int n)  const { return (n + pos_l) * m1 / l1 - pos_m; }
  inline int stage1_out(int n) const { return ((pos_m + n) * l1 + m1 - 1) / m1 - (pos_m * l1 + m1 - 1) / m1; }

  inline void do_stage1(sample_t *in[], sample_t *out[], int n_in, int n_out);
  inline void do_stage2();
  inline void drop_pre_samples();

  void reset_resample();
  size_t process_resample(sample_t *in_buf[], size_t nsamples);
  bool flush_resample();

protected:
  int sample_rate;       // destination sample rate

  Speakers  out_spk;     // output format
  samples_t out_samples; // output buffer
  int       out_size;    // output number of samples

#if RESAMPLE_PERF
public:
  CPUMeter stage1;
  CPUMeter stage2;
#endif

public:
  Resample();
  Resample(int sample_rate, double a = 100, double q = 0.99);
  ~Resample();

  /////////////////////////////////////////////////////////
  // Own interface

  bool set(int sample_rate, double a = 100, double q = 0.99);
  void get(int *sample_rate, double *a = 0, double *q = 0);

  bool set_sample_rate(int _sample_rate) { return set(_sample_rate, a, q); }
  int get_sample_rate() const { return sample_rate; };

  bool set_attenuation(double _a) { return set(sample_rate, _a, q); }
  double get_attenuation() const { return a; }

  bool set_quality(double _q) { return set(sample_rate, a, _q); }
  double get_quality() const { return q; }

  /////////////////////////////////////////////////////////
  // Filter interface

  virtual void reset();
  virtual bool set_input(Speakers spk);
  virtual Speakers get_output() const;
  virtual bool get_chunk(Chunk *chunk);
};

#endif
