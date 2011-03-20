/*
  AudioProcessor PCM passthrough test
  AudioProcessor should not alter input stream with default settings

  Test all possible input formats
  Test standard speaker configs
  Use 48000 sample rate
*/

#include "source/generator.h"
#include "filters/proc.h"
#include "../../suite.h"

static const int seed = 445676;
static const int block_size = 128*1024;

static const int formats[] = 
{ 
  FORMAT_LINEAR, 
  FORMAT_PCM16,    FORMAT_PCM24,    FORMAT_PCM32,
  FORMAT_PCM16_BE, FORMAT_PCM24_BE, FORMAT_PCM32_BE,
};

static const int modes[] = 
{ 
  MODE_1_0,     MODE_2_0,     MODE_3_0,     MODE_2_1,     MODE_3_1,     MODE_2_2,
  MODE_1_0_LFE, MODE_2_0_LFE, MODE_3_0_LFE, MODE_2_1_LFE, MODE_3_1_LFE, MODE_2_2_LFE,
};

///////////////////////////////////////////////////////////////////////////////

TEST(proc_pass, "AudioProcessor noise passthrough test")

  Speakers spk;
  NoiseGen src;
  NoiseGen ref;

  AudioProcessor proc(2048);

  for (int iformat = 0; iformat < array_size(formats); iformat++)
    for (int imode = 0; imode < array_size(modes); imode++)
    {
      spk.set(formats[iformat], modes[imode], 48000);
      log->msg("Testing %s %s %iHz with %iK samples", spk.format_text(), spk.mode_text(), spk.sample_rate, block_size / 1024);

      int data_size = block_size;
      if (spk.format != FORMAT_LINEAR)
        data_size *= spk.nch() * spk.sample_size();

      src.init(spk, seed, data_size);
      ref.init(spk, seed, data_size);

      if (!proc.set_input(spk) || !proc.set_user(spk))
      {
        log->err("Cannot init processor");
        continue;
      }

      compare(log, &src, &proc, &ref);
    }

TEST_END(proc_pass);

///////////////////////////////////////////////////////////////////////////////

SUITE(proc, "AudioProcessor test")
  TEST_FACTORY(proc_pass),
SUITE_END;
