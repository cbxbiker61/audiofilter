#include <math.h>
#include "source/wav_source.h"
#include "sink/sink_wav.h"
#include "filter_graph.h"
#include "filters/agc.h"
#include "filters/convert.h"
#include "filters/gain.h"
#include "vtime.h"
#include "vargs.h"

const int block_size = 65536;

int main(int argc, char **argv)
{
  if (argc < 3)
  {
    printf(

"Gain\n"
"====\n"
"Simple gain tool to amplify or attenuate an audio file.\n"
"This utility is a part of AC3Filter project (http://ac3filter.net)\n"
"Copyright (c) 2008-2009 by Alexander Vigovsky\n"
"\n"
"Usage:\n"
"  > gain input.wav output.wav [-g[ain]:n]\n"
"\n"
"Options:\n"
"  input.wav  - file to process\n"
"  output.wav - file to write the result to\n"
"  -gain - gain to apply\n"
"\n"
"Example:\n"
" > gain a.wav b.wav -gain:-10\n"
" Attenuate by 10dB\n"
    );
    return 0;
  }

  const char *input_filename = argv[1];
  const char *output_filename = argv[2];
  double gain = 1.0;

  /////////////////////////////////////////////////////////////////////////////
  // Parse arguments

  for (int iarg = 3; iarg < argc; iarg++)
  {
    // -gain
    if (is_arg(argv[iarg], "gain", argt_num))
    {
      gain = db2value(arg_num(argv[iarg]));
      continue;
    }

    printf("Error: unknown option: %s\n", argv[iarg]);
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
  // Build the chain

  Converter iconv(block_size);
  Converter oconv(block_size);
  iconv.set_format(FORMAT_LINEAR);
  oconv.set_format(src.get_output().format);
  Gain gain_filter;
  AGC agc(1024);

  gain_filter.gain = gain;

  FilterChain chain;
  chain.add_back(&iconv, "Input converter");
  chain.add_back(&gain_filter, "Gain");
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
