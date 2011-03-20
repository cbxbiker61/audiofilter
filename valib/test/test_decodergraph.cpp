/*
  DecoderGraph filter test
  * dynamical user format change
*/

#include "log.h"
#include "filter_tester.h"
#include "filters\decoder_graph.h"
#include <source\raw_source.h>

///////////////////////////////////////////////////////////////////////////////
// Test class

class DecoderGraph_test
{
protected:
  Filter *f;
  FilterTester t;
  DecoderGraph dec;
  Log *log;

public:
  DecoderGraph_test(Log *_log)
  {
    log = _log;
    t.link(&dec, log);
    f = &dec; // do not use FilterTester
  }

  int test()
  {
    log->open_group("DecoderGraph test");
    user_format_change();
    return log->close_group();
  }

  /////////////////////////////////////////////////////////////////////////////
  // Dynamical user format change test

  void user_format_change()
  {
    log->open_group("Dynamical user format change");

    user_format_change("a.ac3.03f.ac3", Speakers(FORMAT_AC3, MODE_5_1, 48000));
    user_format_change("a.pcm.005.lpcm", Speakers(FORMAT_PCM16_BE, MODE_STEREO, 48000));

    log->close_group();
  }

  int user_format_change(const char *file_name, Speakers spk)
  {
    Speakers formats[] = {
      Speakers(FORMAT_PCM16, 0, 0),                 // pcm16 as-is
      Speakers(FORMAT_PCM16, MODE_STEREO, 0),       // pcm16 stereo (possible downmix)
      Speakers(FORMAT_PCM16, MODE_5_1, 0),          // pcm16 5.1 (possible upmix)
      Speakers(FORMAT_PCM16, 0, 44100),             // pcm16 as-is (possible downsample)
      Speakers(FORMAT_PCM16, MODE_STEREO, 44100),   // pcm16 stereo (possible downmix+downsample)
      Speakers(FORMAT_PCM16, MODE_5_1, 44100),      // pcm16 5.1 (possible upmix+downsample)
      Speakers(FORMAT_PCM16, 0, 48000),             // pcm16 as-is (possible upsample)
      Speakers(FORMAT_PCM16, MODE_STEREO, 48000),   // pcm16 stereo (possible downmix+upsample)
      Speakers(FORMAT_PCM16, MODE_5_1, 48000),      // pcm16 5.1 (possible upmix+upsample)

      Speakers(FORMAT_PCM32, 0, 0),                 // pcm32 as-is
      Speakers(FORMAT_PCM32, MODE_STEREO, 0),       // pcm32 stereo (possible downmix)
      Speakers(FORMAT_PCM32, MODE_5_1, 0),          // pcm32 5.1 (possible upmix)
      Speakers(FORMAT_PCM32, 0, 44100),             // pcm32 as-is (possible downsample)
      Speakers(FORMAT_PCM32, MODE_STEREO, 44100),   // pcm32 stereo (possible downmix+downsample)
      Speakers(FORMAT_PCM32, MODE_5_1, 44100),      // pcm32 5.1 (possible upmix+downsample)
      Speakers(FORMAT_PCM32, 0, 48000),             // pcm32 as-is (possible upsample)
      Speakers(FORMAT_PCM32, MODE_STEREO, 48000),   // pcm32 stereo (possible downmix+upsample)
      Speakers(FORMAT_PCM32, MODE_5_1, 48000),      // pcm32 5.1 (possible upmix+upsample)
    };

    log->open_group("Testing %s (%s %s %i)", file_name, 
      spk.format_text(), spk.mode_text(), spk.sample_rate);

    Chunk chunk;
    RAWSource src(spk, file_name, 2048);

    if (!src.is_open())
      return log->err_close("Cannot open file %s", file_name);

    if (!f->set_input(spk))
      return log->err_close("dec.set_input(%s %s %iHz) failed", 
        spk.format_text(), spk.mode_text(), spk.sample_rate);

    for (int i = 0; i < array_size(formats); i++)
    {
      dec.set_user(formats[i]);

      Speakers test_spk = spk;
      if (formats[i].format)
      {
        test_spk.format = formats[i].format;
        test_spk.level = formats[i].level;
      }

      if (formats[i].mask)
        test_spk.mask = formats[i].mask;

      while (!src.is_empty() && f->get_output() != test_spk)
        if (f->is_empty())
        {
          if (!src.get_chunk(&chunk))
            return log->err_close("src->get_chunk() failed");

          if (!f->process(&chunk))
            return log->err_close("dec.process() failed");
        }
        else
        {
          if (!f->get_chunk(&chunk))
            return log->err_close("dec.get_chunk() failed");
        }

      if (f->get_output() != test_spk)
        return log->err_close("cannot change format: %s %s -> %s %s",
          formats[i].format? formats[i].format_text(): "as-is",
          formats[i].mask? formats[i].mode_text(): "as-is",
          test_spk.format_text(), test_spk.mode_text());
      else
        log->msg("successful format change: %s %s -> %s %s",
          formats[i].format? formats[i].format_text(): "as-is",
          formats[i].mask? formats[i].mode_text(): "as-is",
          test_spk.format_text(), test_spk.mode_text());
    }

    return log->close_group();
  }

};

///////////////////////////////////////////////////////////////////////////////
// Test function

int test_decodergraph(Log *log)
{
  DecoderGraph_test test(log);
  return test.test();
}
