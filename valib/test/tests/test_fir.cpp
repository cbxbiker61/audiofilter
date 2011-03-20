/*
  * Base FIR classes test:
    * Base FIR instances
    * Base FIR generators
  * FIRRef class test
  * ParamFIR test using Convolver filter
*/

#include "source/generator.h"
#include "filters/convolver.h"
#include "filters/gain.h"
#include "filters/slice.h"

#include "fir.h"
#include "fir/delay_fir.h"
#include "fir/param_fir.h"
#include "fir/multi_fir.h"
#include "fir/parallel_fir.h"
#include "../suite.h"

#include <memory>

static const Speakers spk = Speakers(FORMAT_LINEAR, MODE_STEREO, 48000);
static const size_t noise_size = 64 * 1024;
static const int seed = 94586;

///////////////////////////////////////////////////////////////////////////////
// Base FIR classes test
///////////////////////////////////////////////////////////////////////////////

TEST(fir_base, "Base FIR classes test")

  const int sample_rate = 48000;
  const double gain = 0.5;

  // Base FIR instance classes test

  const ZeroFIRInstance zero_inst(sample_rate);
  const IdentityFIRInstance identity_inst(sample_rate);
  const GainFIRInstance gain_inst(sample_rate, gain);

  CHECK(zero_inst.sample_rate == sample_rate);
  CHECK(identity_inst.sample_rate == sample_rate);
  CHECK(gain_inst.sample_rate == sample_rate);

  CHECK(zero_inst.type == firt_zero);
  CHECK(identity_inst.type == firt_identity);
  CHECK(gain_inst.type == firt_gain);

  CHECK(zero_inst.length == 1);
  CHECK(identity_inst.length == 1);
  CHECK(gain_inst.length == 1);

  CHECK(zero_inst.data[0] == 0.0);
  CHECK(identity_inst.data[0] == 1.0);
  CHECK(gain_inst.data[0] == gain);

  // Base FIR generators test

  FIRZero zero_gen;
  FIRIdentity identity_gen;
  FIRGain gain_gen(gain);

  const FIRInstance *fir_ptr;

  fir_ptr = zero_gen.make(sample_rate);
  CHECK(fir_ptr != 0);
  CHECK(fir_ptr->sample_rate == sample_rate);
  CHECK(fir_ptr->type == firt_zero);
  CHECK(fir_ptr->length == 1);
  CHECK(fir_ptr->data[0] == 0.0);
  safe_delete(fir_ptr);

  fir_ptr = identity_gen.make(sample_rate);
  CHECK(fir_ptr != 0);
  CHECK(fir_ptr->sample_rate == sample_rate);
  CHECK(fir_ptr->type == firt_identity);
  CHECK(fir_ptr->length == 1);
  CHECK(fir_ptr->data[0] == 1.0);
  safe_delete(fir_ptr);

  fir_ptr = gain_gen.make(sample_rate);
  CHECK(fir_ptr != 0);
  CHECK(fir_ptr->sample_rate == sample_rate);
  CHECK(fir_ptr->type == firt_gain);
  CHECK(fir_ptr->length == 1);
  CHECK(fir_ptr->data[0] == gain);
  safe_delete(fir_ptr);

  // Gain generator test

  int ver;

  ver = gain_gen.version();
  gain_gen.set_gain(0.0);
  CHECK(ver != gain_gen.version());
  CHECK(gain_gen.get_gain() == 0.0);

  fir_ptr = gain_gen.make(sample_rate);
  CHECK(fir_ptr->type == firt_zero);
  CHECK(fir_ptr->sample_rate == sample_rate);
  CHECK(fir_ptr->data[0] == 0.0);
  safe_delete(fir_ptr);

  ver = gain_gen.version();
  gain_gen.set_gain(1.0);
  CHECK(ver != gain_gen.version());
  CHECK(gain_gen.get_gain() == 1.0);

  fir_ptr = gain_gen.make(sample_rate);
  CHECK(fir_ptr->type == firt_identity);
  CHECK(fir_ptr->sample_rate == sample_rate);
  CHECK(fir_ptr->data[0] == 1.0);
  safe_delete(fir_ptr);

  ver = gain_gen.version();
  gain_gen.set_gain(gain);
  CHECK(ver != gain_gen.version());
  CHECK(gain_gen.get_gain() == gain);

  fir_ptr = gain_gen.make(sample_rate);
  CHECK(fir_ptr->type == firt_gain);
  CHECK(fir_ptr->sample_rate == sample_rate);
  CHECK(fir_ptr->data[0] == gain);
  safe_delete(fir_ptr);

TEST_END(fir_base);

///////////////////////////////////////////////////////////////////////////////
// FIRRef class test
///////////////////////////////////////////////////////////////////////////////

TEST(fir_ref, "FIRRef class test")

  const int sample_rate = 48000;
  const double gain = 0.5;

  FIRZero zero_gen;
  FIRGain gain_gen(gain);

  int ver;
  const FIRInstance *fir_ptr;

  // Default constructor test

  FIRRef ref;
  ver = ref.version();
  CHECK(ref.get() == 0);
  CHECK(ref.make(sample_rate) == 0);

  // Init constructor test

  FIRRef ref2(&zero_gen);
  ver = ref2.version();
  CHECK(ref2.get() == &zero_gen);

  // Copy constructor test

  FIRRef ref3(ref2);
  ver = ref3.version();
  CHECK(ref3.get() == &zero_gen);

  // Generator change test

  ver = ref.version();
  ref.set(&zero_gen);
  CHECK(ref.version() != ver);
  CHECK(ref.get() == &zero_gen);

  ver = ref.version();
  ref.set(&gain_gen);
  CHECK(ref.version() != ver);
  CHECK(ref.get() == &gain_gen);

  // Assignment test

  ver = ref.version();
  ref = ref2;
  CHECK(ref.version() != ver);
  CHECK(ref.get() == &zero_gen);

  // Generation test

  fir_ptr = ref.make(sample_rate);
  CHECK(fir_ptr != 0);
  CHECK(fir_ptr->type == firt_zero);
  safe_delete(fir_ptr);

  // Release test

  ver = ref.version();
  ref.release();
  CHECK(ref.version() != ver);
  CHECK(ref.get() == 0);

  fir_ptr = ref.make(sample_rate);
  CHECK(fir_ptr == 0);

TEST_END(fir_ref)

///////////////////////////////////////////////////////////////////////////////
// ParamFIR class test
//
// Convolve sine wave with parametric filter and watch that it is remain
// unchanged or filtered out depending on its frequency and filter parameters.
///////////////////////////////////////////////////////////////////////////////

TEST(param_fir, "ParamFIR")
  const double att = 100; // 100dB attenuation
  const int trans = 100; // 100Hz transition bandwidth
  double diff, level;
  int len;

  const FIRInstance *fir;

  // FIRs

  int freq = spk.sample_rate / 4;
  ParamFIR low_pass(FIR_LOW_PASS, freq, 0, trans, att);
  ParamFIR high_pass(FIR_HIGH_PASS, freq, 0, trans, att);
  ParamFIR band_pass(FIR_BAND_PASS, freq - trans, freq + trans, trans, att);
  ParamFIR band_stop(FIR_BAND_STOP, freq - trans, freq + trans, trans, att);

  // Test source test_src (tone -> convolver -> slice)

  ToneGen tone;
  SliceFilter slice;
  Convolver conv;
  SourceFilter conv_src(&tone, &conv);
  SourceFilter test_src(&conv_src, &slice);

  // Reference source ref_src (tone -> slice)

  ToneGen ref_tone;
  SliceFilter ref_slice;
  SourceFilter ref_src(&ref_tone, &ref_slice);

  /////////////////////////////////////////////////////////
  // Low pass
  /////////////////////////////////////////////////////////

  fir = low_pass.make(spk.sample_rate);
  CHECK(fir != 0);
  len = 2 * fir->length;
  delete fir;

  conv.set_fir(&low_pass);

  /////////////////////////////////////////////////////////
  // Tone in the pass band must remain unchanged

  tone.init(spk, freq - trans, 0, noise_size + 2 * len);
  slice.init(len, noise_size + len);
  ref_tone.init(spk, freq - trans, 0, noise_size + 2 * len);
  ref_slice.init(len, len + noise_size);
  conv.reset();

  diff = calc_diff(&test_src, &ref_src);
  CHECK(diff > 0);
  CHECK(value2db(diff) < -att);

  /////////////////////////////////////////////////////////
  // Tone in the stop band must be filtered out

  tone.init(spk, freq + trans, 0, noise_size + 2 * len);
  slice.init(len, noise_size + len);
  conv.reset();

  level = calc_peak(&test_src);
  CHECK(level > 0);
  CHECK(value2db(level) < -att);

  /////////////////////////////////////////////////////////
  // High pass
  /////////////////////////////////////////////////////////

  fir = high_pass.make(spk.sample_rate);
  CHECK(fir != 0);
  len = 2 * fir->length;
  delete fir;

  conv.set_fir(&high_pass);

  /////////////////////////////////////////////////////////
  // Tone in the pass band must remain unchanged

  tone.init(spk, freq + trans, 0, noise_size + 2 * len);
  slice.init(len, noise_size + len);
  ref_tone.init(spk, freq + trans, 0, noise_size + 2 * len);
  ref_slice.init(len, len + noise_size);
  conv.reset();

  diff = calc_diff(&test_src, &ref_src);
  CHECK(diff > 0);
  CHECK(value2db(diff) < -att);

  /////////////////////////////////////////////////////////
  // Tone in the stop band must be filtered out

  tone.init(spk, freq - trans, 0, noise_size + 2 * len);
  slice.init(len, noise_size + len);
  conv.reset();

  level = calc_peak(&test_src);
  CHECK(level > 0);
  CHECK(value2db(level) < -att);

  /////////////////////////////////////////////////////////
  // BandPass
  /////////////////////////////////////////////////////////

  fir = band_pass.make(spk.sample_rate);
  CHECK(fir != 0);
  len = 2 * fir->length;
  delete fir;

  conv.set_fir(&band_pass);

  /////////////////////////////////////////////////////////
  // Tone in the pass band must remain unchanged

  tone.init(spk, freq, 0, noise_size + 2 * len);
  slice.init(len, noise_size + len);
  ref_tone.init(spk, freq, 0, noise_size + 2 * len);
  ref_slice.init(len, len + noise_size);
  conv.reset();

  diff = calc_diff(&test_src, &ref_src);
  CHECK(diff > 0);
  CHECK(value2db(diff) < -att);

  /////////////////////////////////////////////////////////
  // Tones at stop bands must be filtered out

  tone.init(spk, freq - 2 * trans, 0, noise_size + 2 * len);
  slice.init(len, noise_size + len);
  conv.reset();

  level = calc_peak(&test_src);
  CHECK(level > 0);
  CHECK(value2db(level) < -att);

  tone.init(spk, freq + 2 * trans, 0, noise_size + 2 * len);
  slice.init(len, noise_size + len);
  conv.reset();

  level = calc_peak(&test_src);
  CHECK(level > 0);
  CHECK(value2db(level) < -att);

  /////////////////////////////////////////////////////////
  // BandStop
  /////////////////////////////////////////////////////////

  fir = band_stop.make(spk.sample_rate);
  CHECK(fir != 0);
  len = 2 * fir->length;
  delete fir;

  conv.set_fir(&band_stop);

  /////////////////////////////////////////////////////////
  // Tones at pass bands must remain unchanged

  tone.init(spk, freq - 2 * trans, 0, noise_size + 2 * len);
  slice.init(len, noise_size + len);
  ref_tone.init(spk, freq - 2 * trans, 0, noise_size + 2 * len);
  ref_slice.init(len, len + noise_size);
  conv.reset();

  diff = calc_diff(&test_src, &ref_src);
  CHECK(diff > 0);
  CHECK(value2db(diff) < -att);

  tone.init(spk, freq + 2 * trans, 0, noise_size + 2 * len);
  slice.init(len, noise_size + len);
  ref_tone.init(spk, freq + 2 * trans, 0, noise_size + 2 * len);
  ref_slice.init(len, len + noise_size);
  conv.reset();

  diff = calc_diff(&test_src, &ref_src);
  CHECK(diff > 0);
  CHECK(value2db(diff) < -att);

  /////////////////////////////////////////////////////////
  // Tone in the stop band must be filtered out

  tone.init(spk, freq, 0, noise_size + 2 * len);
  slice.init(len, noise_size + len);
  conv.reset();

  level = calc_peak(&test_src);
  CHECK(level > 0);
  CHECK(value2db(level) < -att);

TEST_END(param_fir);

///////////////////////////////////////////////////////////////////////////////
// MultiFIR class test
//
// * Combine 2 gains and get gain, identity and zero results
// * Combine ParamFIR with GainFIR and ShiftFIR and get original ParamFIR, but
// gained and shifted.
///////////////////////////////////////////////////////////////////////////////

TEST(multi_fir, "MultiFIR test")
  int sample_rate = 48000;
  const int freq = 8000;   // 8kHz frequency for low-pass filter
  const double att = 50;   // 50dB atteniutaion
  const int trans = 500;   // 500Hz transition bandwidth
  const double gain = 0.5; // 1/2 gain
  const int delay = 10;    // delay in samples;

  const FIRInstance *fir;
  const FIRInstance *fir_ref;
  int i, ver;

  ParamFIR low_pass(FIR_LOW_PASS, freq, 0, trans, att);
  DelayFIR delay_fir(vtime_t(delay) / sample_rate);
  FIRGain gain_fir(gain);

  const FIRGen *list_zero[] = { &fir_zero };
  const FIRGen *list_identity[] = { &fir_identity };
  const FIRGen *list[] = { &low_pass, 0, &gain_fir, &delay_fir };

  /////////////////////////////////////////////////////////
  // Default constructor

  MultiFIR f;
  fir = f.make(sample_rate);
  CHECK(fir == 0);

  /////////////////////////////////////////////////////////
  // Init constructor test

  MultiFIR f2(list_zero, array_size(list_zero));
  fir = f2.make(sample_rate);
  CHECK(fir != 0);
  CHECK(fir->type == firt_zero);
  safe_delete(fir);

  /////////////////////////////////////////////////////////
  // Combine 2 gains

  FIRGain gain1;
  FIRGain gain2;
  const FIRGen *gain_list[] = { &gain1, &gain2 };

  f.set(gain_list, array_size(gain_list));

  gain1.set_gain(1.0);
  gain2.set_gain(2.0);

  fir = f.make(sample_rate);
  CHECK(fir != 0);
  CHECK(fir->type == firt_gain);
  CHECK(fir->data[0] == 2.0);
  safe_delete(fir);

  gain1.set_gain(2.0);
  gain2.set_gain(0.5);

  fir = f.make(sample_rate);
  CHECK(fir != 0);
  CHECK(fir->type == firt_identity);
  safe_delete(fir);

  gain1.set_gain(2.0);
  gain2.set_gain(0.0);

  fir = f.make(sample_rate);
  CHECK(fir != 0);
  CHECK(fir->type == firt_zero);
  safe_delete(fir);

  /////////////////////////////////////////////////////////
  // set() function test

  ver = f.version();
  f.set(list_zero, array_size(list_zero));
  CHECK(ver != f.version());

  fir = f.make(sample_rate);
  CHECK(fir != 0);
  CHECK(fir->type == firt_zero);
  safe_delete(fir);

  ver = f.version();
  f.set(list_identity, array_size(list_identity));
  CHECK(ver != f.version());

  fir = f.make(sample_rate);
  CHECK(fir != 0);
  CHECK(fir->type == firt_identity);
  safe_delete(fir);

  /////////////////////////////////////////////////////////
  // Multi filters test

  f.set(list, array_size(list));
  fir = f.make(sample_rate);
  fir_ref = low_pass.make(sample_rate);
  CHECK(fir != 0);
  CHECK(fir_ref != 0);
  CHECK(fir->length == fir_ref->length + delay);

  for (i = 0; i < delay; i++)
    CHECK(fir->data[i] == 0.0);

  for (i = 0; i < fir_ref->length; i++)
    CHECK(EQUAL_SAMPLES(fir_ref->data[i] * gain, fir->data[i+delay]));

TEST_END(multi_fir);

///////////////////////////////////////////////////////////////////////////////
// ParallelFIR class test
//
// * Combine 2 gains and get gain, identity and zero results
// * Sum HPF, LPF, Delay, Zero and Gain filters get Echo result
///////////////////////////////////////////////////////////////////////////////

TEST(parallel_fir, "ParallelFIR test")
  int sample_rate = 48000;
  const int freq = 8000;   // 8kHz frequency for low/high-pass filter
  const double att = 50;   // 50dB atteniutaion
  const int trans = 500;   // 500Hz transition bandwidth
  const int delay = 10;    // delay in samples;
  const double gain = 0.5; // 1/2 gain

  const FIRInstance *fir;
  int i, ver;

  ParamFIR low_pass(FIR_LOW_PASS, freq, 0, trans, att);
  ParamFIR high_pass(FIR_HIGH_PASS, freq, 0, trans, att);
  DelayFIR delay_fir(vtime_t(delay) / sample_rate);
  FIRGain gain_fir(gain);

  const FIRGen *list_zero[] = { &fir_zero };
  const FIRGen *list_identity[] = { &fir_identity };
  const FIRGen *list[] = { &low_pass, 0, &high_pass, &fir_zero, &gain_fir, &delay_fir };

  /////////////////////////////////////////////////////////
  // Default constructor

  ParallelFIR f;
  fir = f.make(sample_rate);
  CHECK(fir == 0);

  /////////////////////////////////////////////////////////
  // Init constructor test

  ParallelFIR f2(list_zero, array_size(list_zero));
  fir = f2.make(sample_rate);
  CHECK(fir != 0);
  CHECK(fir->type == firt_zero);
  safe_delete(fir);

  /////////////////////////////////////////////////////////
  // set() function test

  ver = f.version();
  f.set(list_zero, array_size(list_zero));
  CHECK(ver != f.version());

  fir = f.make(sample_rate);
  CHECK(fir != 0);
  CHECK(fir->type == firt_zero);
  safe_delete(fir);

  ver = f.version();
  f.set(list_identity, array_size(list_identity));
  CHECK(ver != f.version());

  fir = f.make(sample_rate);
  CHECK(fir != 0);
  CHECK(fir->type == firt_identity);
  safe_delete(fir);

  /////////////////////////////////////////////////////////
  // Combine 2 gains

  FIRGain gain1;
  FIRGain gain2;
  const FIRGen *gain_list[] = { &gain1, &gain2 };

  f.set(gain_list, array_size(gain_list));

  gain1.set_gain(1.0);
  gain2.set_gain(2.0);

  fir = f.make(sample_rate);
  CHECK(fir != 0);
  CHECK(fir->type == firt_gain);
  CHECK(fir->data[0] == 3.0);
  safe_delete(fir);

  gain1.set_gain(0.5);
  gain2.set_gain(0.5);

  fir = f.make(sample_rate);
  CHECK(fir != 0);
  CHECK(fir->type == firt_identity);
  safe_delete(fir);

  gain1.set_gain(0.0);
  gain2.set_gain(0.0);

  fir = f.make(sample_rate);
  CHECK(fir != 0);
  CHECK(fir->type == firt_zero);
  safe_delete(fir);

  /////////////////////////////////////////////////////////
  // Multi filters test

  f.set(list, array_size(list));
  fir = f.make(sample_rate);
  CHECK(fir != 0);

  for (i = 0; i < fir->length; i++)
    if (i != fir->center && i != fir->center + delay)
      CHECK(fir->data[i] <= SAMPLE_THRESHOLD / fir->length);

  CHECK(EQUAL_SAMPLES(fir->data[fir->center], gain + 1.0));
  CHECK(EQUAL_SAMPLES(fir->data[fir->center + delay], 1.0));

TEST_END(parallel_fir);

///////////////////////////////////////////////////////////////////////////////
// Test suite
///////////////////////////////////////////////////////////////////////////////

SUITE(fir, "FIR tests")
  TEST_FACTORY(fir_base),
  TEST_FACTORY(fir_ref),
  TEST_FACTORY(param_fir),
  TEST_FACTORY(multi_fir),  
  TEST_FACTORY(parallel_fir),  
SUITE_END;
