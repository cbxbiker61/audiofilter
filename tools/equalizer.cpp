#include "source/wav_source.h"
#include "sink/sink_wav.h"
#include "filter_graph.h"
#include "filters/agc.h"
#include "filters/convert.h"
#include "filters/dither.h"
#include "filters/equalizer.h"
#include "vtime.h"
#include "vargs.h"

const int block_size = 65536;
const int max_bands = 100;

int main(int argc, char **argv)
{
  int i;

  if (argc < 3)
  {
    printf(

"Equalizer\n"
"======\n"
"Simple graphic equalizer\n"
"This utility is a part of AC3Filter project (http://ac3filter.net)\n"
"Copyright (c) 2008-2009 by Alexander Vigovsky\n"
"\n"
"Usage:\n"
"  > equalizer input.wav output.wav [-fx:n] [-gx:n]\n"
"\n"
"Options:\n"
"  input.wav  - file to process\n"
"  output.wav - file to write the result to\n"
"  -fx - center frequency in Hz for band x (100 bands max)\n"
"  -gx - gain in dB for band x (100 bands max)\n"
"  -dither - dither the result\n"
"\n"
"If gain for a band specified with -fx parameter is not set, 0dB is assumed\n"
"\n"
"Example:\n"
" > equalizer a.wav b.wav -f1:100 -g1:-6 -f2:200 -g2:3 -f3:500\n"
" Attenuate all frequencies below 100Hz by 6dB, gain the band with the center\n"
" at 200Hz by 3dB, and leave everything after 500Hz unchanged\n"
    );
    return 0;
  }

  const char *input_filename = argv[1];
  const char *output_filename = argv[2];
  EqBand bands[max_bands];
  bool do_dither = false;

  for (i = 0; i < max_bands; i++) bands[i].freq = 0, bands[i].gain = 0;

  /////////////////////////////////////////////////////////////////////////////
  // Parse arguments

  for (int iarg = 3; iarg < argc; iarg++)
  {
    // -dither
    if (is_arg(argv[iarg], "dither", argt_bool))
    {
      do_dither = arg_bool(argv[iarg]);
      continue;
    }
    else
    {
      // -fx -gx
      for (i = 0; i < max_bands; i++)
      {
        char buf[10];
        sprintf(buf, "f%i", i);
        if (is_arg(argv[iarg], buf, argt_num))
        {
          bands[i].freq = (int)arg_num(argv[iarg]);
          break;
        }

        sprintf(buf, "g%i", i);
        if (is_arg(argv[iarg], buf, argt_num))
        {
          if (bands[i].freq == 0)
          {
            printf("Unknown freqency for band %i (define the band's frequency before the gain)\n", i);
            return -1;
          }
          bands[i].gain = arg_num(argv[iarg]);
          break;
        }
      }

      if (i < max_bands)
        continue;
    }

    printf("Error: unknown option: %s\n", argv[iarg]);
    return -1;
  }

  /////////////////////////////////////////////////////////////////////////////
  // Compact bands and print band info

  int nbands = 0;
  for (i = 0; i < max_bands; i++)
    if (bands[i].freq != 0)
    {
      bands[nbands].freq = bands[i].freq;
      bands[nbands].gain = bands[i].gain;
      nbands++;
    }

  if (nbands < max_bands)
  {
    bands[nbands].freq = 0;
    bands[nbands].gain = 0;
  }

  if (nbands == 0)
  {
    printf("No bands set\n");
    return -1;
  }

  printf("%i bands:\n", nbands);
  for (i = 0; i < nbands; i++)
    printf("%i Hz => %g dB\n", bands[i].freq, bands[i].gain);

  for (i = 0; i < nbands; i++)
    bands[i].gain = db2value(bands[i].gain);

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
  // Build the chain

  Converter iconv(block_size);
  Converter oconv(block_size);
  iconv.set_format(FORMAT_LINEAR);
  oconv.set_format(src.get_output().format);

  Equalizer eq;
  eq.set_enabled(true);
  if (!eq.set_bands(bands, nbands))
  {
    printf("Bad band parameters\n");
    return -1;
  }

  AGC agc(1024);
  Dither dither;
  FilterChain chain;

  chain.add_back(&iconv, "Input converter");
  chain.add_back(&eq, "Equalizer");
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

  /////////////////////////////////////////////////////////////////////////////
  // Do the job

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
