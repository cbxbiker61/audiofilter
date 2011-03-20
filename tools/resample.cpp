#include <stdio.h>
#include "filter_graph.h"
#include "filters\agc.h"
#include "filters\convert.h"
#include "filters\dither.h"
#include "filters\resample.h"
#include "source\wav_source.h"
#include "sink\sink_wav.h"
#include "vtime.h"
#include "vargs.h"

const int block_size = 65536;

int main(int argc, char **argv)
{
  if (argc < 3)
  {
    printf(
"Resample\n"
"========\n"
"Sample rate conversion utility\n"
"This utility is a part of AC3Filter project (http://ac3filter.net)\n"
"Copyright (c) 2008-2009 by Alexander Vigovsky\n"
"\n"
"Usage:\n"
"  > resample input.wav output.wav [-r[ate]:n] [-q[uality]:n] [-a[ttenuation]:n] [-d[ither]]\n"
"\n"
"Options:\n"
"  input.wav  - file to convert\n"
"  output.wav - file to write the result to\n"
"  rate - new sample rate (deafult: 48000)\n"
"  quality - passband width (default: 0.99)\n"
"  attenuation - stopband attenuation in dB (default: 100)\n"
"  dither - dither the result\n"
"\n"
"Example:\n"
"  > resample input.wav output.wav -r:44100 -q:0.999 -a:150\n"
"  Passband width in this example is 0.999. This means that only uppper 22Hz\n"
"  of the destination bandwidth (22028Hz - 22050Hz) will be distorted.\n"
"  Note, that attenuation is large here. Such attenuation is only sensible for\n"
"  at least 24bit files.\n");
    return 0;
  }

  const char *input_filename = argv[1];
  const char *output_filename = argv[2];
  int sample_rate = 48000;
  double q = 0.99;
  double a = 100;
  bool do_dither = false;

  for (int iarg = 3; iarg < argc; iarg++)
  {
    // -r[ate]
    if (is_arg(argv[iarg], "r", argt_num) || 
        is_arg(argv[iarg], "rate", argt_num))
    {
      sample_rate = (int)arg_num(argv[iarg]);
      continue;
    }

    // -q[uality]
    if (is_arg(argv[iarg], "q", argt_num) || 
        is_arg(argv[iarg], "quality", argt_num))
    {
      q = arg_num(argv[iarg]);
      continue;
    }

    // -a[ttenuation]
    if (is_arg(argv[iarg], "a", argt_num) || 
        is_arg(argv[iarg], "attenuation", argt_num))
    {
      a = arg_num(argv[iarg]);
      continue;
    }

    // -d[ither]
    if (is_arg(argv[iarg], "d", argt_bool) || 
        is_arg(argv[iarg], "dither", argt_bool))
    {
      do_dither = arg_bool(argv[iarg]);
      continue;
    }

    printf("Error: unknown option: %s\n", argv[iarg]);
    return 1;
  }

  if (sample_rate == 0)
  {
    printf("Error: incorrect sample rate\n");
    return -1;
  }

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

  Converter iconv(block_size);
  Converter oconv(block_size);
  Resample resample;
  AGC agc(1024);
  Dither dither;

  iconv.set_format(FORMAT_LINEAR);
  oconv.set_format(src.get_output().format);
  if (!resample.set(sample_rate, a, q))
  {
    printf("Error: cannot do sample rate conversion with given parameters\n", output_filename);
    return -1;
  }

  FilterChain chain;
  chain.add_back(&iconv, "Input converter");
  chain.add_back(&resample, "Resample");
  if (do_dither && !spk.is_floating_point())
  {
    chain.add_back(&dither, "Dither");
    dither.level = 0.5 / spk.level;
  }
  chain.add_back(&agc, "AGC");
  chain.add_back(&oconv, "Output converter");

  Chunk chunk;
  printf("Conversion %i -> %i\n", src.get_output().sample_rate, sample_rate);
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
