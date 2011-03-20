#include <math.h>
#include "source/generator.h"
#include "filters/convolver_mch.h"
#include "filters/gain.h"
#include "filters/slice.h"
#include "fir/param_fir.h"
#include "../../suite.h"

static const Speakers spk = Speakers(FORMAT_LINEAR, MODE_QUADRO, 48000);
static const size_t noise_size = 64 * 1024;
static const int seed = 37753234;

TEST(convolver_mch, "ConvolverMch test")

  int ch_name;
  sample_t level, diff;

  const double gain = 0.5;
  const int freq = 1000;
  const int df = 100;
  const double att = 100;
  const int *order = spk.order();

  FIRZero zero_fir;
  FIRIdentity identity_fir;
  FIRGain gain_fir(gain);

  ParamFIR lpf1(FIR_LOW_PASS, freq, 0, df / 2, att);
  ParamFIR lpf2(FIR_LOW_PASS, freq, 0, df, att);

  NoiseGen noise1;
  NoiseGen noise2;
  ZeroGen zero;

  Chunk chunk;
  ConvolverMch conv;
  Gain gain_filter(gain);

  // test_src: tone -> conv -> slice
  // ref_src: ref_tone -> ref_slice

  ToneGen tone;
  SliceFilter slice;
  SourceFilter conv_src(&tone, &conv);
  SourceFilter test_src(&conv_src, &slice);

  ToneGen ref_tone;
  SliceFilter ref_slice;
  SourceFilter ref_src(&ref_tone, &ref_slice);

  // multichannel generators

  const FIRGen *null_all[NCHANNELS];
  const FIRGen *zero_all[NCHANNELS];
  const FIRGen *pass_all[NCHANNELS];
  const FIRGen *gain_all[NCHANNELS];
  const FIRGen *lpf_all[NCHANNELS];

  for (ch_name = 0; ch_name < NCHANNELS; ch_name++)
  {
    null_all[ch_name] = 0;
    zero_all[ch_name] = &zero_fir;
    pass_all[ch_name] = &identity_fir;
    gain_all[ch_name] = &gain_fir;
    lpf_all[ch_name] = &lpf1;
  }

  // determine lpf filter length

  const FIRInstance *fir = lpf1.make(spk.sample_rate);
  CHECK(fir != 0);
  size_t filter_len = 2 * fir->length;
  delete fir;

  /////////////////////////////////////////////////////////
  // Change FIR

  for (ch_name = 0; ch_name < NCHANNELS; ch_name++)
  {
    conv.set_fir(ch_name, &zero_fir);
    CHECK(conv.get_fir(ch_name) == &zero_fir);

    conv.set_fir(ch_name, &identity_fir);
    CHECK(conv.get_fir(ch_name) == &identity_fir);
  }

  conv.set_all_firs(zero_all);
  for (ch_name = 0; ch_name < NCHANNELS; ch_name++)
    CHECK(conv.get_fir(ch_name) == &zero_fir);
  
  conv.set_all_firs(pass_all);
  for (ch_name = 0; ch_name < NCHANNELS; ch_name++)
    CHECK(conv.get_fir(ch_name) == &identity_fir);

  /////////////////////////////////////////////////////////
  // Convolve with null response

  noise1.init(spk, seed, noise_size);
  noise2.init(spk, seed, noise_size);

  conv.set_all_firs(null_all);
  conv.reset();

  CHECK(compare(log, &noise1, &conv, &noise2, 0) == 0);

  /////////////////////////////////////////////////////////
  // Convolve with zero response

  noise1.init(spk, seed, noise_size);
  zero.init(spk, noise_size);

  conv.set_all_firs(zero_all);
  conv.reset();

  CHECK(compare(log, &noise1, &conv, &zero, 0) == 0);

  /////////////////////////////////////////////////////////
  // Convolve with identity response

  noise1.init(spk, seed, noise_size);
  noise2.init(spk, seed, noise_size);

  conv.set_all_firs(pass_all);
  conv.reset();

  CHECK(compare(log, &noise1, &conv, &noise2, 0) == 0);

  /////////////////////////////////////////////////////////
  // Convolve with gain response

  noise1.init(spk, seed, noise_size);
  noise2.init(spk, seed, noise_size);

  conv.set_all_firs(gain_all);
  conv.reset();

  CHECK(compare(log, &noise1, &conv, &noise2, &gain_filter) == 0);

  /////////////////////////////////////////////////////////
  // Convolve with low-pass filter

  // Tone in the pass band must remain unchanged

  tone.init(spk, freq - df, 0, noise_size + 2 * filter_len);
  slice.init(filter_len, noise_size + filter_len);
  ref_tone.init(spk, freq - df, 0, noise_size + 2 * filter_len);
  ref_slice.init(filter_len, filter_len + noise_size);
  conv.set_all_firs(lpf_all);
  conv.reset();

  diff = calc_diff(&test_src, &ref_src);
  CHECK(diff > 0);
  CHECK(value2db(diff) < -att);

  // Tone in the stop band must be filtered out

  tone.init(spk, freq + df, 0, noise_size + 2 * filter_len);
  slice.init(filter_len, noise_size + filter_len);
  conv.set_all_firs(lpf_all);
  conv.reset();

  level = calc_peak(&test_src);
  CHECK(level > 0);
  CHECK(value2db(level) < -att);

  /////////////////////////////////////////////////////////
  // Mix firs

  // lpf1, lpf2, identity and null filters produce identity output

  assert(spk.mask & MODE_QUADRO);
  const FIRGen *mix_pass[NCHANNELS];
  for (ch_name = 0; ch_name < NCHANNELS; ch_name++)
    mix_pass[ch_name] = 0;

  mix_pass[CH_L] = &lpf1;
  mix_pass[CH_R] = &lpf2;
  mix_pass[CH_SL] = 0;
  mix_pass[CH_SR] = &identity_fir;

  tone.init(spk, freq - df, 0, noise_size + 2 * filter_len);
  slice.init(filter_len, noise_size + filter_len);
  ref_tone.init(spk, freq - df, 0, noise_size + 2 * filter_len);
  ref_slice.init(filter_len, filter_len + noise_size);
  conv.set_all_firs(mix_pass);
  conv.reset();

  diff = calc_diff(&test_src, &ref_src);
  CHECK(diff > 0);
  CHECK(value2db(diff) < -att);

  // lpf1, lpf2 and zero filters produce zero output

  assert(spk.mask & MODE_QUADRO);
  const FIRGen *mix_zero[NCHANNELS];
  for (ch_name = 0; ch_name < NCHANNELS; ch_name++)
    mix_zero[ch_name] = &zero_fir;

  mix_zero[CH_L] = &lpf1;
  mix_zero[CH_R] = &lpf2;
  mix_zero[CH_SL] = &zero_fir;
  mix_pass[CH_SR] = &zero_fir;

  tone.init(spk, freq + df, 0, noise_size + 2 * filter_len);
  slice.init(filter_len, noise_size + filter_len);
  conv.set_all_firs(mix_zero);
  conv.reset();

  level = calc_peak(&test_src);
  CHECK(level > 0);
  CHECK(value2db(level) < -att);


  /////////////////////////////////////////////////////////
  // Change FIR on the fly
  // TODO

TEST_END(convolver_mch);
