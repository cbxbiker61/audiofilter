#include "source/wav_source.h"
#include "sink/sink_wav.h"
#include "filter_graph.h"
#include "filters/agc.h"
#include "filters/convert.h"
#include "filters/convolver.h"
#include "filters/dither.h"
#include "fir/param_fir.h"
#include "vtime.h"
#include "vargs.h"

const int block_size = 65536;

int main(int argc, char **argv)
{
  if (argc < 3)
  {
    printf(

"Filter\n"
"======\n"
"Simple parametric audio filter\n"
"This utility is a part of AC3Filter project (http://ac3filter.net)\n"
"Copyright (c) 2008-2009 by Alexander Vigovsky\n"
"\n"
"Usage:\n"
"  > filter input.wav output.wav -<type> -f:n [-f2:n] -df:n [-a:n] [-norm]\n"
"\n"
"Options:\n"
"  input.wav  - file to process\n"
"  output.wav - file to write the result to\n"
"  -<type> - filter type:\n"
"    -lowpass  - low pass filter\n"
"    -highpass - high pass filter\n"
"    -bandpass - band pass filter\n"
"    -bandstop - band stop filter\n"
"  -f    - center frequency of the filter (all filters)\n"
"  -f2   - second bound frequency (bandpass and bandstop filters only)\n"
"  -df   - transition band width\n"
"  -a    - attenuation factor in dB (100dB by default)\n"
"  -norm - if this switch is present all frequecies are specified in\n"
"          normalized form instead of Hz.\n"
"  -dither - dither the result\n"
"\n"
"Examples:\n"
"  Band-pass filter from 100Hz to 8kHz with transition width of 100Hz.\n"
"  Attenuate all other frequencies (0-50Hz and 8050 to nyquist) by 100dB.\n"
"  > filter in.wav out.wav -bandpass -f:100 -f2:8000 -df:100 -a:100\n"
"\n"
"  Low-pass half of the total bandwidth with 1%% transition bandwidth.\n"
"  Note, that in normalized form nyquist frequency is 0.5 (not 1.0!), so half\n"
"  of the bandwidth is 0.25 and 1% of the bandwidth is 0.005.\n"
"  For 48kHz input, this means low-pass filter with 12kHz cutoff frequency\n"
"  and 240Hz transition band width.\n"
"  > filter in.wav out.wav -lowpass -f:0.25 -df:0.005\n"

    );
    return 0;
  }

  const char *input_filename = argv[1];
  const char *output_filename = argv[2];
  double f = 0;
  double f2 = 0;
  double df = 0;
  double a = 100;
  bool norm = false;
  bool do_dither = false;
  int type = -1;

  /////////////////////////////////////////////////////////////////////////////
  // Parse arguments

  for (int iarg = 3; iarg < argc; iarg++)
  {
    // -<type>
    if (is_arg(argv[iarg], "lowpass", argt_exist))
    {
      if (type != -1)
      {
        printf("Filter type is ambigous\n");
        return -1;
      }
      type = FIR_LOW_PASS;
      continue;
    }

    if (is_arg(argv[iarg], "highpass", argt_exist))
    {
      if (type != -1)
      {
        printf("Filter type is ambigous\n");
        return -1;
      }
      type = FIR_HIGH_PASS;
      continue;
    }

    if (is_arg(argv[iarg], "bandpass", argt_exist))
    {
      if (type != -1)
      {
        printf("Filter type is ambigous\n");
        return -1;
      }
      type = FIR_BAND_PASS;
      continue;
    }

    if (is_arg(argv[iarg], "bandstop", argt_exist))
    {
      if (type != -1)
      {
        printf("Filter type is ambigous\n");
        return -1;
      }
      type = FIR_BAND_STOP;
      continue;
    }

    // -f
    if (is_arg(argv[iarg], "f", argt_num))
    {
      f = arg_num(argv[iarg]);
      continue;
    }

    // -f2
    if (is_arg(argv[iarg], "f2", argt_num))
    {
      f2 = arg_num(argv[iarg]);
      continue;
    }

    // -df
    if (is_arg(argv[iarg], "df", argt_num))
    {
      df = arg_num(argv[iarg]);
      continue;
    }

    // -a
    if (is_arg(argv[iarg], "a", argt_num))
    {
      a = arg_num(argv[iarg]);
      continue;
    }

    // -norm
    if (is_arg(argv[iarg], "norm", argt_exist))
    {
      norm = true;
      continue;
    }

    // -dither
    if (is_arg(argv[iarg], "dither", argt_exist))
    {
      do_dither = true;
      continue;
    }

    printf("Error: unknown option: %s\n", argv[iarg]);
    return -1;
  }

  /////////////////////////////////////////////////////////////////////////////
  // Validate arguments

  if (type == -1)
  {
    printf("Please, specify the filter type\n");
    return -1;
  }

  if (f == 0)
  {
    printf("Please, specify the filter frequency with -f option\n");
    return -1;
  }

  if (type == FIR_BAND_PASS || type == FIR_BAND_STOP) if (f2 == 0)
  {
    printf("Please, specify the second bound frequency with -f2 option\n");
    return -1;
  }

  if (df == 0)
  {
    printf("Please, specify the trnasition band width with -df option\n");
    return -1;
  }

  /////////////////////////////////////////////////////////////////////////////
  // Open files

  WAVSource src(input_filename, block_size);
  if (!src.is_open())
  {
    printf("Error: cannot open file: %s\n", input_filename);
    return -1;
  }

  WAVSink sink(output_filename);
  if (!sink.is_open())
  {
    printf("Error: cannot open file: %s\n", output_filename);
    return -1;
  }

  Speakers spk = src.get_output();

  /////////////////////////////////////////////////////////////////////////////
  // Print processing info

  const char *type_text = "Unknown";
  switch (type)
  {
    case FIR_LOW_PASS:  type_text = "Low-pass"; break;
    case FIR_HIGH_PASS: type_text = "High-pass"; break;
    case FIR_BAND_PASS: type_text = "Band-pass"; break;
    case FIR_BAND_STOP: type_text = "Band-stop"; break;
  };

  if (type == FIR_BAND_PASS || type == FIR_BAND_STOP)
    if (norm) printf("Filter: %s\nFirst bound freq: %.8g (%.8g Hz)\nSecound bound freq: %.8g (%.8g Hz)\nTransition band width: %.8g (%.8g Hz)\n", type_text, f, f * spk.sample_rate , f2, f2 * spk.sample_rate, df, df * spk.sample_rate);
    else printf("Filter: %s\nFirst bound freq: %.8g Hz\nSecound bound freq: %.8g Hz\nTransition band width: %.8g Hz\n", type_text, f, f2, df);
  else
    if (norm) printf("Filter: %s\nBound freq: %.8g (%.8g Hz)\nTransition band width: %.8g (%.8g Hz)\n", type_text, f, f * spk.sample_rate, df, df * spk.sample_rate);
    else printf("Filter: %s\nBound freq: %.8g Hz\nTransition band width: %.8g Hz\n", type_text, f, df);
  printf("Attenuation: %.8g dB\n", a);

  /////////////////////////////////////////////////////////////////////////////
  // Init FIR and print its info

  ParamFIR fir(type, f, f2, df, a, norm);
  const FIRInstance *data = fir.make(spk.sample_rate);

  if (!data)
  {
    printf("Error in filter parameters!\n");
    return -1;
  }

  printf("Filter length: %i\n", data->length);
  delete data;


  /////////////////////////////////////////////////////////////////////////////
  // Do the job

  Converter iconv(block_size);
  Converter oconv(block_size);
  iconv.set_format(FORMAT_LINEAR);
  oconv.set_format(src.get_output().format);

  Convolver filter(&fir);
  AGC agc(1024);
  Dither dither;

  FilterChain chain;
  chain.add_back(&iconv, "Input converter");
  chain.add_back(&filter, "Convolver");
  if (do_dither && !spk.is_floating_point())
  {
    chain.add_back(&dither, "Dither");
    dither.level = 0.5 / spk.level;
  }
  chain.add_back(&agc, "AGC");
  chain.add_back(&oconv, "Output converter");

  if (!chain.set_input(spk))
  {
    printf("Error: cannot start processing\n");
    return -1;
  }

  Chunk chunk;
  printf("0%%\r");

  vtime_t t = local_time() + 0.1;
  while (!src.is_empty())
  {
    src.get_chunk(&chunk);
    if (!chain.process_to(&chunk, &sink))
    {
      char buf[1024];
      chain.chain_text(buf, array_size(buf));
      printf("Processing error. Chain dump: %s\n", buf);
      return -1;
    }

    if (local_time() > t)
    {
      t += 0.1;
      double pos = double(src.pos()) * 100 / src.size();
      printf("%i%%\r", (int)pos);
    }
  }
  printf("100%%\n");
  return 0;
}
