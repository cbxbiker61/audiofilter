/*
  Resample filter test
  * Test that backward transform is equal to the bandlimited original signal
*/

#include "source/generator.h"
#include "filters/resample.h"
#include "filters/convolver.h"
#include "filters/slice.h"
#include "filter_graph.h"
#include "fir/param_fir.h"
#include "../../suite.h"


static const int seed = 485706;
static const int block_size = 64 * 1024;

static const int transform_rates[][2] =
{
  { 192000, 11025 }, // 2560/147
  { 192000,  4000 }, // 48/1

  { 176400,  6000 }, // 147/5
  { 176400,  4000 }, // 441/10

  { 128000, 11025 }, // 5120/441
  { 128000,  6000 }, // 64/3
  { 128000,  4000 }, // 32/1

  {  96000, 11025 }, // 1280/147
  {  96000,  4000 }, // 24/1

  {  88200,  6000 }, // 147/10
  {  88200,  4000 }, // 441/20

  {  64000, 11025 }, // 2560/441
  {  64000,  6000 }, // 32/3
  {  64000,  4000 }, // 16/1

  {  48000, 44100 }, // 160/147
  {  48000, 32000 }, // 3/2
  {  48000, 24000 }, // 2/1
  {  48000, 22050 }, // 320/147
  {  48000, 16000 }, // 3/1
  {  48000, 12000 }, // 4/1
  {  48000, 11025 }, // 640/147
  {  48000,  8000 }, // 6/1
  {  48000,  6000 }, // 8/1
  {  48000,  4000 }, // 12/1

  {  44100, 32000 }, // 441/320
  {  44100, 24000 }, // 147/80
  {  44100, 16000 }, // 441/160
  {  44100, 12000 }, // 147/40
  {  44100,  8000 }, // 441/80
  {  44100,  6000 }, // 147/20
  {  44100,  4000 }, // 441/40

  {  32000, 24000 }, // 4/3
  {  32000, 22050 }, // 640/441
  {  32000, 12000 }, // 8/3
  {  32000, 11025 }, // 1280/441
  {  32000,  6000 }, // 16/3
};

///////////////////////////////////////////////////////////////////////////////
// Resample reverse transform test
// Resample is reversible when:
// * we have no frequencies above the pass band
// * we throw out transient processes at stream ends

TestResult up_down(Log *log, int rate1, int rate2, double a, double q);

TEST(resample_reverse, "Resample reverse transform test")
  const double q = 0.99; // quality
  const double a = 106;  // attenuation

  // upsample -> downsample
  for (int i = 0; i < array_size(transform_rates); i++)
    up_down(log, transform_rates[i][0], transform_rates[i][1], a, q);

TEST_END(resample_reverse);


TestResult up_down(Log *log, int rate1, int rate2, double a, double q)
{
  if (rate1 > rate2)
  {
    int temp = rate1;
    rate1 = rate2;
    rate2 = temp;
  }

  /////////////////////////////////////////////////////////////////////////////
  // After resample only q*nyquist of the bandwidth is preserved. Therefore,
  // to compare output of the resampler with the original signal we must feed
  // the resampler with the bandlimited signal. Bandlimiting filter has a
  // transition band and we must guarantee:
  // passband + transition_band <= q*nyquist.
  //
  // It looks reasonable to make the transition band of the bandlimiting filter
  // to be equal to the transition band of the resampler. In this case we have:
  // passband + (1-q)*nyquist <= q*nyquist
  // passband <= (2q - 1)*nyquist
  //
  // In normalized form nyquist = 0.5, so we have following parameters of the
  // bandlimiting filter: passband = q-0.5, transition band = 0.5*(1-q)

  ParamFIR low_pass(FIR_LOW_PASS, q-0.5, 0, 0.5*(1-q), a + 10, true);

  const FIRInstance *fir = low_pass.make(rate1);
  CHECK(fir != 0);
  int trans_len = fir->length * 2;
  delete fir;

  Speakers spk(FORMAT_LINEAR, MODE_STEREO, rate1);
  NoiseGen tst_noise(spk, seed, block_size + trans_len * 2);
  NoiseGen ref_noise(spk, seed, block_size + trans_len * 2);
  Convolver tst_conv(&low_pass);
  Convolver ref_conv(&low_pass);
  Resample res1(rate2, a, q);
  Resample res2(rate1, a, q);
  SliceFilter tst_slice(trans_len, block_size - trans_len);
  SliceFilter ref_slice(trans_len, block_size - trans_len);

  FilterChain tst_chain;
  tst_chain.add_back(&tst_conv, "Bandlimit");
  tst_chain.add_back(&res1, "Upsample");
  tst_chain.add_back(&res2, "Downsample");
  tst_chain.add_back(&tst_slice, "Slice");

  FilterChain ref_chain;
  ref_chain.add_back(&ref_conv, "Bandlimit");
  ref_chain.add_back(&ref_slice, "Slice");

  // Resample introduces not more than -A dB of noise.
  // 2 resamples introduces twice more noise, -A + 6dB
  double diff = calc_diff(&tst_noise, &tst_chain, &ref_noise, &ref_chain);
  log->msg("Transform: %5iHz -> %6iHz -> %5iHz Diff: %.0fdB", rate1, rate2, rate1, log10(diff) * 20);
  CHECK(diff > 0);
  CHECK(log10(diff) * 20 <= -a + 7);

  return test_passed;
}

SUITE(resample, "Resample filter test")
  TEST_FACTORY(resample_reverse),
SUITE_END;
