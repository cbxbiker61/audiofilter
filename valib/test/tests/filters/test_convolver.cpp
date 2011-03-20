#include <math.h>
#include "source/generator.h"
#include "filters/convolver.h"
#include "filters/gain.h"
#include "filters/slice.h"
#include "fir/param_fir.h"
#include "../../suite.h"

static const Speakers spk = Speakers(FORMAT_LINEAR, MODE_STEREO, 48000);
static const size_t noise_size = 64 * 1024;
static const int seed = 123123;

TEST(convolver, "Convolver test")

  const double gain = 0.5;
  Gain gain_filter(gain);

  FIRZero zero_fir;
  FIRIdentity identity_fir;
  FIRGain gain_fir(gain);

  NoiseGen noise1;
  NoiseGen noise2;
  ZeroGen zero;

  Chunk chunk;

  // Default constructor

  Convolver conv;
  CHECK(conv.get_fir() == 0);

  // Init constructor

  Convolver conv1(&zero_fir);
  CHECK(conv1.get_fir() == &zero_fir);

  /////////////////////////////////////////////////////////
  // Change FIR

  conv.set_fir(&zero_fir);
  CHECK(conv.get_fir() == &zero_fir);

  conv.set_fir(&identity_fir);
  CHECK(conv.get_fir() == &identity_fir);

  /////////////////////////////////////////////////////////
  // Convolve with null response

  noise1.init(spk, seed, noise_size);
  noise2.init(spk, seed, noise_size);

  conv.set_fir(0);
  conv.reset();

  CHECK(compare(log, &noise1, &conv, &noise2, 0) == 0);

  /////////////////////////////////////////////////////////
  // Convolve with zero response

  noise1.init(spk, seed, noise_size);
  zero.init(spk, noise_size);

  conv.set_fir(&zero_fir);
  conv.reset();

  CHECK(compare(log, &noise1, &conv, &zero, 0) == 0);

  /////////////////////////////////////////////////////////
  // Convolve with identity response

  noise1.init(spk, seed, noise_size);
  noise2.init(spk, seed, noise_size);

  conv.set_fir(&identity_fir);
  conv.reset();

  CHECK(compare(log, &noise1, &conv, &noise2, 0) == 0);

  /////////////////////////////////////////////////////////
  // Convolve with gain response

  noise1.init(spk, seed, noise_size);
  noise2.init(spk, seed, noise_size);

  conv.set_fir(&gain_fir);
  conv.reset();

  CHECK(compare(log, &noise1, &conv, &noise2, &gain_filter) == 0);

  /////////////////////////////////////////////////////////
  // Convolve with low-pass filter

  sample_t level, diff;
  size_t filter_len;
  const int freq = 1000;
  const int df = 100;
  const double att = 100;
  ParamFIR low_pass(FIR_LOW_PASS, freq, 0, df, att);

  const FIRInstance *fir = low_pass.make(spk.sample_rate);
  CHECK(fir != 0);
  filter_len = 2 * fir->length;
  delete fir;

  ToneGen tone;
  SliceFilter slice;
  SourceFilter conv_src(&tone, &conv);
  SourceFilter test_src(&conv_src, &slice);

  ToneGen ref_tone;
  SliceFilter ref_slice;
  SourceFilter ref_src(&ref_tone, &ref_slice);

  // Tone in the pass band must remain unchanged

  tone.init(spk, freq - df, 0, noise_size + 2 * filter_len);
  slice.init(filter_len, noise_size + filter_len);
  ref_tone.init(spk, freq - df, 0, noise_size + 2 * filter_len);
  ref_slice.init(filter_len, filter_len + noise_size);
  conv.set_fir(&low_pass);
  conv.reset();

  diff = calc_diff(&test_src, &ref_src);
  CHECK(diff > 0);
  CHECK(value2db(diff) < -att);

  // Tone in the stop band must be filtered out

  tone.init(spk, freq + df, 0, noise_size + 2 * filter_len);
  slice.init(filter_len, noise_size + filter_len);
  conv.reset();

  level = calc_peak(&test_src);
  CHECK(level > 0);
  CHECK(value2db(level) < -att);

  /////////////////////////////////////////////////////////
  // Change FIR on the fly
  // TODO

TEST_END(convolver);
