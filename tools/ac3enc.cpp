#include <cstdio>

#include <source/wav_source.h>
#include <sink/sink_raw.h>
#include <parsers/ac3/ac3_enc.h>
#include <filters/convert.h>
#include <filter_graph.h>
//#include <win32/cpu.h>
#include <vargs.h>

int main(int argc, char *argv[])
{
  if (argc < 3)
  {
    printf(
"AC3 Encoder\n"
"===========\n"
"This utility is a part of AC3Filter project (http://ac3filter.net)\n"
"Copyright (c) 2007-2009 by Alexander Vigovsky\n"
"\n"
"Usage:\n"
"  ac3enc input.wav output.ac3 [-br:bitrate]\n"
    );
    return 1;
  }

  /////////////////////////////////////////////////////////
  // Parse arguments
  /////////////////////////////////////////////////////////

  const char *input_filename = argv[1];
  const char *output_filename = argv[2];
  int bitrate = 448000;

  for (int iarg = 3; iarg < argc; iarg++)
  {
    if (is_arg(argv[iarg], "br", argt_num))
    {
       bitrate = (int)arg_num(argv[iarg]);
       continue;
    }

    printf("Error: unknown option: %s\n", argv[iarg]);
    return 1;
  }

  /////////////////////////////////////////////////////////
  // Open files
  /////////////////////////////////////////////////////////

  WAVSource src;
  if (!src.open(input_filename, 65536))
  {
    printf("Cannot open file %s (not a PCM file?)\n", input_filename);
    return 1;
  }

  RAWSink sink;
  if (!sink.open(output_filename))
  {
    printf("Cannot open file %s for writing!\n", argv[2]);
    return 1;
  }

  /////////////////////////////////////////////////////////
  // Setup everything
  /////////////////////////////////////////////////////////

  Converter   conv(2048);
  AC3Enc      enc;
  FilterChain chain;

  chain.add_back(&conv, "Converter");
  chain.add_back(&enc,  "Encoder");

  conv.set_format(FORMAT_LINEAR);
  conv.set_order(win_order);

  if (!enc.set_bitrate(bitrate))
  {
    printf("Wrong bitrate (%i)!\n", bitrate);
    return 1;
  }

  Speakers spk = src.get_output();
  spk.format = FORMAT_LINEAR;
  if (!enc.set_input(spk))
  {
    printf("Cannot encode this file (%s %iHz)!\n", spk.sample_rate);
    return 1;
  }

  /////////////////////////////////////////////////////////
  // Process
  /////////////////////////////////////////////////////////

  Chunk raw_chunk;
  Chunk ac3_chunk;

  CPUMeter cpu_usage;
  CPUMeter cpu_total;

  double ms = 0;
  double old_ms = 0;

  cpu_usage.start();
  cpu_total.start();

  fprintf(stderr, "0.0%% Frs/err: 0/0\tTime: 0:00.000i\tFPS: 0 CPU: 0%%\r");
  int frames = 0;
  while (!src.is_empty())
  {
    if (!src.get_chunk(&raw_chunk))
    {
      printf("Data load error!\n");
      return 1;
    }

    if (!chain.process(&raw_chunk))
    {
      printf("Encoding error!\n");
      return 1;
    }

    while (!chain.is_empty())
    {
      if (!chain.get_chunk(&ac3_chunk))
      {
        printf("Encoding error!\n");
        return 1;
      }

      sink.process(&ac3_chunk);
      frames++;

      /////////////////////////////////////////////////////
      // Statistics

      ms = double(cpu_total.get_system_time() * 1000);
      if (ms > old_ms + 100)
      {
        old_ms = ms;

        // Statistics
        fprintf(stderr, "%2.1f%% Frames: %i\tTime: %i:%02i.%03i\tFPS: %i CPU: %.1f%%  \r",
          double(src.pos()) * 100.0 / src.size(),
          frames,
          int(ms/60000), int(ms) % 60000/1000, int(ms) % 1000,
          int(frames * 1000 / (ms+1)),
          cpu_usage.usage() * 100);
      } // if (ms > old_ms + 100)
    }
  }

  fprintf(stderr, "%2.1f%% Frames: %i\tTime: %i:%02i.%03i\tFPS: %i CPU: %.1f%%  \n",
    double(src.pos()) * 100.0 / src.size(),
    frames,
    int(ms/60000), int(ms) % 60000/1000, int(ms) % 1000,
    int(frames * 1000 / (ms+1)),
    cpu_usage.usage() * 100);

  cpu_usage.stop();
  cpu_total.stop();

  printf("System time: %ims\n", int(cpu_total.get_system_time() / 10000));
  printf("Process time: %ims\n", int(cpu_total.get_thread_time() / 10000));

  return 0;
}
