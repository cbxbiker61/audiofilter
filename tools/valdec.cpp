#include <stdio.h>
//#include <conio.h>
#include <math.h>

// parsers
#include <parsers/file_parser.h>

#include <parsers/ac3/ac3_header.h>
#include <parsers/dts/dts_header.h>
#include <parsers/mpa/mpa_header.h>
#include <parsers/spdif/spdif_header.h>
#include <parsers/multi_header.h>

// sinks
#include <sink/sink_raw.h>
#include <sink/sink_wav.h>
//#include <sink/sink_dsound.h>

// filters
#include <filters/dvd_graph.h>

// other
//#include <win32/cpu.h>
#include <vargs.h>


#define bool2str(v) ((v)? "true": "false")
#define bool2str1(v) ((v)? "+": "-")


const char *_ch_names[NCHANNELS] = { "Left", "Center", "Right", "Left surround", "Right surround", "LFE" };


const int mask_tbl[] =
{
  0,
  MODE_MONO,
  MODE_STEREO,
  MODE_3_0,
  MODE_2_2,
  MODE_3_2,
  MODE_5_1
};

const int format_tbl[] =
{
  FORMAT_PCM16,
  FORMAT_PCM24,
  FORMAT_PCM32,
  FORMAT_PCM16_BE,
  FORMAT_PCM24_BE,
  FORMAT_PCM32_BE,
  FORMAT_PCMFLOAT,
};

const sample_t level_tbl[] =
{
  32767,
  8388607,
  2147483647,
  1.0,
  32767,
  8388607,
  2147483647,
  1.0
};

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    printf(

"Valib decoder\n"
"=============\n"
"Audio decoder/processor/player utility\n"
"\n"
"This utility is a part of AC3Filter project (http://ac3filter.net)\n"
"Copyright (c) 2006-2009 by Alexander Vigovsky\n"
"\n"
"Usage:\n"
"  valdec some_file [options]\n"
"\n"
"Options:\n"
"  (*)  - default value\n"
"  {ch} - channel name (l, c, r, sl, sr)\n"
"\n"
"  output mode:\n"
"    -d[ecode]  - just decode (used for testing and performance measurements)\n"
"    -p[lay]    - play file (*)\n"
"    -r[aw] file.raw - decode to RAW file\n"
"    -w[av] file.wav - decode to WAV file\n"
"    -n[othing] - do nothing (to be used with -i option)\n"
"  \n"
"  output options:\n"
//"    -spdif - spdif output (no other options will work in this mode)\n"
"    -spk:n - set number of output channels:\n"
"      (*) 0 - from file     4 - 2/2 (quadro)\n"
"          1 - 1/0 (mono)    5 - 3/2 (5 ch)\n"
"          2 - 2/0 (stereo)  6 - 3/2+SW (5.1)\n"
"          3 - 3/0 (surround)\n"
"    -fmt:n - set sample format:\n"
"      (*) 0 - PCM 16        3 - PCM 16 (big endian)\n"
"          1 - PCM 24        4 - PCM 24 (big endian)\n"
"          2 - PCM 32        5 - PCM 32 (big endian)\n"
"                 6 - PCM Float\n"
"  \n"
"  format selection:\n"
"    -ac3 - force ac3 (do not autodetect format)\n"
"    -dts - force dts (do not autodetect format)\n"
"    -mpa - force mpa (do not autodetect format)\n"
"  \n"
"  info:\n"
"    -i    - print bitstream info\n"
//"  -opt  - print decoding options\n"
"    -hist - print levels histogram\n"
"  \n"
"  mixer options:\n"
"    -auto_matrix[+|-] - automatic matrix calculation on(*)/off\n"
"    -normalize_matrix[+|-] - normalize matrix on(*)/off\n"
"    -voice_control[+|-] - voice control on(*)/off\n"
"    -expand_stereo[+|-] - expand stereo on(*)/off\n"
"    -clev:N - center mix level (dB)\n"
"    -slev:N - surround mix level (dB)\n"
"    -lfelev:N - lfe mix level (dB)\n"
"    -gain:N - master gain (dB)\n"
"    -gain_{ch}:N - output channel gain (dB)\n"
"  \n"
"  automatic gain control options:\n"
"    -agc[+|-] - auto gain control on(*)/off\n"
"    -normalize[+|-] - one-pass normalize on/off(*)\n"
"    -drc[+|-] - dynamic range compression on/off(*)\n"
"    -drc_power:N - dynamic range compression level (dB)\n"
"    -attack:N - attack speed (dB/s)\n"
"    -release:N - release speed (dB/s)\n"
"  \n"
"  delay options:\n"
"    -delay_units:n - units in wich delay values are given\n"
"          0 - samples (*) 2 - meters      4 - feet   \n"
"          1 - ms          3 - cm          5 - inches \n"
"    -delay_{ch}:N - delay for channel {ch} (in samples by default)\n"

           );
    return 1;
  }

  int i;
  bool print_info = false;
  bool print_opt  = false;
  bool print_hist = false;

  /////////////////////////////////////////////////////////
  // Input file

  const char *input_filename = argv[1];
  FileParser file;

  /////////////////////////////////////////////////////////
  // Parsers

  const HeaderParser *parser_list[] = { &spdif_header, &ac3_header, &dts_header, &mpa_header };
  MultiHeader multi_parser(parser_list, array_size(parser_list));

  const HeaderParser *parser = 0;

  /////////////////////////////////////////////////////////
  // Arrays

  int delay_units = DELAY_SP;
  float delays[NCHANNELS];
  for (i = 0; i < NCHANNELS; i++)
    delays[i] = 0;

  sample_t gains[NCHANNELS];
  for (i = 0; i < NCHANNELS; i++)
    gains[i] = 1.0;

  /////////////////////////////////////////////////////////
  // Output format

  int iformat = 0;
  int imask = 0;

  /////////////////////////////////////////////////////////
  // Sinks

  enum { mode_undefined, mode_nothing, mode_play, mode_raw, mode_wav, mode_decode } mode = mode_undefined;
  const char *out_filename = 0;

  RAWSink    raw;
  WAVSink    wav;
  DSoundSink dsound;
  NullSink   null;

  Sink *sink = 0;
  PlaybackControl *control = 0;

  /////////////////////////////////////////////////////////
  // Filters
  /////////////////////////////////////////////////////////

  DVDGraph dvd_graph;

  /////////////////////////////////////////////////////////
  // Parse arguments
  /////////////////////////////////////////////////////////

  for (int iarg = 2; iarg < argc; iarg++)
  {
    ///////////////////////////////////////////////////////
    // Parsers
    ///////////////////////////////////////////////////////

    // -ac3 - force ac3 parser (do not autodetect format)
    if (is_arg(argv[iarg], "ac3", argt_exist))
    {
      if (parser)
      {
        printf("-ac3 : ambigous parser\n");
        return 1;
      }

      parser = &ac3_header;
      continue;
    }

    // -dts - force dts parser (do not autodetect format)
    if (is_arg(argv[iarg], "dts", argt_exist))
    {
      if (parser)
      {
        printf("-dts : ambigous parser\n");
        return 1;
      }

      parser = &dts_header;
      continue;
    }

    // -mpa - force mpa parser (do not autodetect format)
    if (is_arg(argv[iarg], "mpa", argt_exist))
    {
      if (parser)
      {
        printf("-mpa : ambigous parser\n");
        return 1;
      }

      parser = &mpa_header;
      continue;
    }

    ///////////////////////////////////////////////////////
    // Output format
    ///////////////////////////////////////////////////////

    // -spdif - enable SPDIF output
//    if (is_arg(argv[iarg], "spdif", argt_exist))
//    {
//      spdif = true;
//      continue;
//    }

    // -spk - number of speakers
    if (is_arg(argv[iarg], "spk", argt_num))
    {
      imask = int(arg_num(argv[iarg]));

      if (imask < 0 || imask > array_size(mask_tbl))
      {
        printf("-spk : incorrect speaker configuration\n");
        return 1;
      }
      continue;
    }

    // -fmt - sample format
    if (is_arg(argv[iarg], "fmt", argt_num))
    {
      iformat = int(arg_num(argv[iarg]));
      if (iformat < 0 || iformat > array_size(format_tbl))
      {
        printf("-fmt : incorrect sample format");
        return 1;
      }
      continue;
    }

    ///////////////////////////////////////////////////////
    // Sinks
    ///////////////////////////////////////////////////////

    // -d[ecode] - decode
    if (is_arg(argv[iarg], "d", argt_exist) ||
        is_arg(argv[iarg], "decode", argt_exist))
    {
      if (sink)
      {
        printf("-decode : ambigous output mode\n");
        return 1;
      }

      sink = &null;
      control = 0;
      mode = mode_decode;
      continue;
    }

    // -p[lay] - play
    if (is_arg(argv[iarg], "p", argt_exist) ||
        is_arg(argv[iarg], "play", argt_exist))
    {
      if (sink)
      {
        printf("-play : ambigous output mode\n");
        return 1;
      }

      sink = &dsound;
      control = &dsound;
      mode = mode_play;
      continue;
    }

    // -r[aw] - RAW output
    if (is_arg(argv[iarg], "r", argt_exist) ||
        is_arg(argv[iarg], "raw", argt_exist))
    {
      if (sink)
      {
        printf("-raw : ambigous output mode\n");
        return 1;
      }
      if (argc - iarg < 1)
      {
        printf("-raw : specify a file name\n");
        return 1;
      }

      out_filename = argv[++iarg];
      sink = &raw;
      control = 0;
      mode = mode_raw;
      continue;
    }

    // -w[av] - WAV output
    if (is_arg(argv[iarg], "w", argt_exist) ||
        is_arg(argv[iarg], "wav", argt_exist))
    {
      if (sink)
      {
        printf("-wav : ambigous output mode\n");
        return 1;
      }
      if (argc - iarg < 1)
      {
        printf("-wav : specify a file name\n");
        return 1;
      }

      out_filename = argv[++iarg];
      sink = &wav;
      control = 0;
      mode = mode_wav;
      continue;
    }

    // -n[othing] - no output
    if (is_arg(argv[iarg], "n", argt_exist) ||
        is_arg(argv[iarg], "nothing", argt_exist))
    {
      if (sink)
      {
        printf("-nothing : ambigous output mode\n");
        return 1;
      }

      sink = &null;
      control = 0;
      mode = mode_nothing;
      continue;
    }

    ///////////////////////////////////////////////////////
    // Info
    ///////////////////////////////////////////////////////

    // -i - print bitstream info
    if (is_arg(argv[iarg], "i", argt_exist))
    {
      print_info = true;
      continue;
    }

    // -opt - print decoding options
    if (is_arg(argv[iarg], "opt", argt_exist))
    {
      print_opt = true;
      continue;
    }

    // -hist - print levels histogram
    if (is_arg(argv[iarg], "hist", argt_exist))
    {
      print_hist = true;
      continue;
    }

    ///////////////////////////////////////////////////////
    // Mixer options
    ///////////////////////////////////////////////////////

    // -auto_matrix
    if (is_arg(argv[iarg], "auto_matrix", argt_bool))
    {
      dvd_graph.proc.set_auto_matrix(arg_bool(argv[iarg]));
      continue;
    }

    // -normalize_matrix
    if (is_arg(argv[iarg], "normalize_matrix", argt_bool))
    {
      dvd_graph.proc.set_normalize_matrix(arg_bool(argv[iarg]));
      continue;
    }

    // -voice_control
    if (is_arg(argv[iarg], "voice_control", argt_bool))
    {
      dvd_graph.proc.set_voice_control(arg_bool(argv[iarg]));
      continue;
    }

    // -expand_stereo
    if (is_arg(argv[iarg], "expand_stereo", argt_bool))
    {
      dvd_graph.proc.set_expand_stereo(arg_bool(argv[iarg]));
      continue;
    }

    // -clev
    if (is_arg(argv[iarg], "clev", argt_num))
    {
      dvd_graph.proc.set_clev(db2value(arg_num(argv[iarg])));
      continue;
    }

    // -slev
    if (is_arg(argv[iarg], "slev", argt_num))
    {
      dvd_graph.proc.set_slev(db2value(arg_num(argv[iarg])));
      continue;
    }

    // -lfelev
    if (is_arg(argv[iarg], "lfelev", argt_num))
    {
      dvd_graph.proc.set_lfelev(db2value(arg_num(argv[iarg])));
      continue;
    }

    // -gain
    if (is_arg(argv[iarg], "gain", argt_num))
    {
      dvd_graph.proc.set_master(db2value(arg_num(argv[iarg])));
      continue;
    }

    // -gain_l
    if (is_arg(argv[iarg], "gain_l", argt_num))
    {
      gains[CH_L] = db2value(arg_num(argv[iarg]));
      continue;
    }
    // -gain_c
    if (is_arg(argv[iarg], "gain_c", argt_num))
    {
      gains[CH_C] = db2value(arg_num(argv[iarg]));
      continue;
    }
    // -gain_r
    if (is_arg(argv[iarg], "gain_r", argt_num))
    {
      gains[CH_R] = db2value(arg_num(argv[iarg]));
      continue;
    }
    // -gain_sl
    if (is_arg(argv[iarg], "gain_sl", argt_num))
    {
      gains[CH_SL] = db2value(arg_num(argv[iarg]));
      continue;
    }
    // -gain_sr
    if (is_arg(argv[iarg], "gain_sr", argt_num))
    {
      gains[CH_SR] = db2value(arg_num(argv[iarg]));
      continue;
    }
    // -gain_lfe
    if (is_arg(argv[iarg], "gain_lfe", argt_num))
    {
      gains[CH_LFE] = db2value(arg_num(argv[iarg]));
      continue;
    }

    ///////////////////////////////////////////////////////
    // Auto gain control options
    ///////////////////////////////////////////////////////

    // -agc
    if (is_arg(argv[iarg], "agc", argt_bool))
    {
      dvd_graph.proc.set_auto_gain(arg_bool(argv[iarg]));
      continue;
    }

    // -normalize
    if (is_arg(argv[iarg], "normalize", argt_bool))
    {
      dvd_graph.proc.set_normalize(arg_bool(argv[iarg]));
      continue;
    }

    // -drc
    if (is_arg(argv[iarg], "drc", argt_bool))
    {
      dvd_graph.proc.set_drc(arg_bool(argv[iarg]));
      continue;
    }

    // -drc_power
    if (is_arg(argv[iarg], "drc_power", argt_num))
    {
      dvd_graph.proc.set_drc(true);
      dvd_graph.proc.set_drc_power(arg_num(argv[iarg]));
      continue;
    }

    // -attack
    if (is_arg(argv[iarg], "attack", argt_num))
    {
      dvd_graph.proc.set_attack(db2value(arg_num(argv[iarg])));
      continue;
    }

    // -release
    if (is_arg(argv[iarg], "release", argt_num))
    {
      dvd_graph.proc.set_release(db2value(arg_num(argv[iarg])));
      continue;
    }

    ///////////////////////////////////////////////////////
    // Delay options
    ///////////////////////////////////////////////////////

    // -delay
    if (is_arg(argv[iarg], "delay", argt_bool))
    {
      dvd_graph.proc.set_delay(arg_bool(argv[iarg]));
      continue;
    }

    // -delay_units
    if (is_arg(argv[iarg], "delay_units", argt_num))
    {
      switch (int(arg_num(argv[iarg])))
      {
        case 0: delay_units = DELAY_SP;
        case 1: delay_units = DELAY_MS;
        case 2: delay_units = DELAY_M;
        case 3: delay_units = DELAY_CM;
        case 4: delay_units = DELAY_FT;
        case 5: delay_units = DELAY_IN;
      }
      continue;
    }

    // -delay_l
    if (is_arg(argv[iarg], "delay_l", argt_num))
    {
      delays[CH_L] = float(arg_num(argv[iarg]));
      continue;
    }
    // -delay_c
    if (is_arg(argv[iarg], "delay_c", argt_num))
    {
      delays[CH_C] = float(arg_num(argv[iarg]));
      continue;
    }
    // -delay_r
    if (is_arg(argv[iarg], "delay_r", argt_num))
    {
      delays[CH_R] = float(arg_num(argv[iarg]));
      continue;
    }
    // -delay_sl
    if (is_arg(argv[iarg], "delay_sl", argt_num))
    {
      delays[CH_SL] = float(arg_num(argv[iarg]));
      continue;
    }
    // -delay_sr
    if (is_arg(argv[iarg], "delay_sr", argt_num))
    {
      delays[CH_SR] = float(arg_num(argv[iarg]));
      continue;
    }
    // -delay_lfe
    if (is_arg(argv[iarg], "delay_lfe", argt_num))
    {
      delays[CH_LFE] = float(arg_num(argv[iarg]));
      continue;
    }

    printf("Error: unknown option: %s\n", argv[iarg]);
    return 1;
  }

  /////////////////////////////////////////////////////////
  // Open input file and load the first frame
  /////////////////////////////////////////////////////////

  if (!parser)
    parser = &multi_parser;

  if (!file.open(input_filename, parser, 1000000))
  {
    printf("Error: Cannot open file '%s'\n", input_filename);
    return 1;
  }

  if (!file.stats())
  {
    printf("Error: Cannot detect input file format\n", input_filename);
    return 1;
  }

  while (!file.eof())
    if (file.load_frame())
      break;

  if (!file.is_frame_loaded())
  {
    printf("Error: Cannot load the first frame\n");
    return 1;
  }

  /////////////////////////////////////////////////////////
  // Print file info
  /////////////////////////////////////////////////////////

  if (print_info)
  {
    char info[1024];
    file.file_info(info, sizeof(info));
    printf("%s\n", info);
    file.stream_info(info, sizeof(info));
    printf("%s", info);
  }

  if (mode == mode_nothing)
    return 0;

  /////////////////////////////////////////////////////////
  // Setup processing
  /////////////////////////////////////////////////////////

  dvd_graph.proc.set_delay_units(delay_units);
  dvd_graph.proc.set_delays(delays);

  dvd_graph.proc.set_output_gains(gains);

  dvd_graph.proc.set_input_order(std_order);
  dvd_graph.proc.set_output_order(win_order);

  Speakers user_spk(format_tbl[iformat], mask_tbl[imask], 0, level_tbl[iformat]);
  if (!dvd_graph.set_user(user_spk))
  {
    printf("Error: unsupported user format (%s %s %i)\n",
      user_spk.format_text(), user_spk.mode_text(), user_spk.sample_rate);
    return 1;
  }

  Speakers in_spk = file.get_spk();
  if (!dvd_graph.set_input(in_spk))
  {
    printf("Error: unsupported input format (%s %s %i)\n",
      in_spk.format_text(), in_spk.mode_text(), in_spk.sample_rate);
    return 1;
  }

  Speakers out_spk = user_spk;
  if (!out_spk.mask)
    out_spk.mask = in_spk.mask;
  if (!out_spk.sample_rate)
    out_spk.sample_rate = in_spk.sample_rate;

  /////////////////////////////////////////////////////////
  // Print processing options
  /////////////////////////////////////////////////////////

  //
  // TODO
  //

  /////////////////////////////////////////////////////////
  // Open output file
  /////////////////////////////////////////////////////////

  if (!sink)
  {
    sink = &dsound;
    control = &dsound;
    mode = mode_play;
  }

  switch (mode)
  {
    case mode_raw:
      if (!out_filename || !raw.open(out_filename))
      {
        printf("Error: failed to open output file '%s'\n", out_filename);
        return 1;
      }
      break;

    case mode_wav:
      if (!out_filename || !wav.open(out_filename))
      {
        printf("Error: failed to open output file '%s'\n", out_filename);
        return 1;
      }
      break;

    case mode_play:
      if (!dsound.open_dsound(0))
      {
        printf("Error: failed to init DirectSound\n");
        return 1;
      }
      break;

    // do nothing for other modes
  }

  /////////////////////////////////////////////////////////
  // Process
  /////////////////////////////////////////////////////////

  Chunk chunk;

  CPUMeter cpu_current;
  CPUMeter cpu_total;

  cpu_current.start();
  cpu_total.start();

  double time = 0;
  double old_time = 0;

  sample_t levels[NCHANNELS];
  sample_t level = 0;

  int streams = 0;

//  fprintf(stderr, " 0.0%% Frs:      0 Err: 0 Time:   0:00.000i Level:    0dB FPS:    0 CPU: 0%%\r");

  file.seek(0);

  #define PRINT_STAT                                                                                           \
  {                                                                                                            \
    if (control)                                                                                               \
    {                                                                                                          \
      dvd_graph.proc.get_output_levels(control->get_playback_time(), levels);                                  \
      level = levels[0];                                                                                       \
      for (i = 1; i < NCHANNELS; i++)                                                                          \
        if (levels[i] > level)                                                                                 \
          level = levels[i];                                                                                   \
    }                                                                                                          \
    fprintf(stderr, "%4.1f%% Frs: %-6i Err: %-i Time: %3i:%02i.%03i Level: %-4idB FPS: %-4i CPU: %.1f%%  \r",  \
      file.get_pos(file.relative) * 100,                                                                       \
      file.get_frames(), dvd_graph.dec.get_errors(),                                                           \
      int(time/60), int(time) % 60, int(time * 1000) % 1000,                                                   \
      int(value2db(level)),                                                                                    \
      int(file.get_frames() / time),                                                                           \
      cpu_current.usage() * 100);                                                                              \
  }

  #define DROP_STAT \
    fprintf(stderr, "                                                                             \r");

  /////////////////////////////////////////////////////
  // Now we already have a frame loaded
  // To avoid resyncing again we will use this frame...

  while (!file.eof())
  {
    if (file.is_frame_loaded())
    {
      /////////////////////////////////////////////////////
      // Switch to a new stream

      if (file.is_new_stream())
      {
        if (streams > 0)
          PRINT_STAT;

        if (streams > 0 && print_info)
        {
          char info[1024];
          file.stream_info(info, sizeof(info));
          printf("\n\n%s", info);
        }

        streams++;
        if (mode == mode_nothing)
          return 0;
      }

      /////////////////////////////////////////////////////
      // Process data

      chunk.set_rawdata(file.get_spk(), file.get_frame(), file.get_frame_size());
      if (!dvd_graph.process(&chunk))
      {
        printf("\nError in dvd_graph.process()\n");
        return 1;
      }

      while (!dvd_graph.is_empty())
      {
        if (!dvd_graph.get_chunk(&chunk))
        {
          printf("\nError in dvd_graph.get_chunk()\n");
          return 1;
        }

        /////////////////////////////////////////////////////
        // Do audio output

        if (!chunk.is_dummy())
        {
          if (chunk.spk != sink->get_input())
          {
            if (sink->query_input(chunk.spk))
            {
              DROP_STAT;
              printf("Opening audio output %s %s %i...\n",
                chunk.spk.format_text(), chunk.spk.mode_text(), chunk.spk.sample_rate);
            }
            else
            {
              printf("\nOutput format %s %s %i is unsupported\n",
                chunk.spk.format_text(), chunk.spk.mode_text(), chunk.spk.sample_rate);
              return 1;
            }
          }

          if (!sink->process(&chunk))
          {
            printf("\nError in sink->process()\n");
            return 1;
          }
        }
      } // while (!dvd_graph.is_empty())

    }

    /////////////////////////////////////////////////////
    // Load next frame

    file.load_frame();

    /////////////////////////////////////////////////////
    // Statistics

    time = cpu_total.get_system_time();
    if (time > old_time + 0.1)
    {
      old_time = time;
      PRINT_STAT;
    }

  } // while (!file.eof())

  /////////////////////////////////////////////////////
  // Flushing

  chunk.set_empty(dvd_graph.get_input());
  chunk.eos = true;

  if (!dvd_graph.process_to(&chunk, sink))
  {
    printf("\nProcessing error!\n");
    return 1;
  }

  /////////////////////////////////////////////////////
  // Stop

  cpu_current.stop();
  cpu_total.stop();

  /////////////////////////////////////////////////////
  // Final statistics

  PRINT_STAT;
  printf("\n---------------------------------------\n");
  if (streams > 1)
    printf("Streams found: %i\n", streams);
  printf("Frames/errors: %i/%i\n", file.get_frames(), dvd_graph.dec.get_errors());
  printf("System time: %ims\n", int(cpu_total.get_system_time() * 1000));
  printf("Process time: %ims\n", int(cpu_total.get_thread_time() * 1000 ));
  printf("Approx. %.2f%% realtime CPU usage\n", double(cpu_total.get_thread_time() * 100) / file.get_size(file.time));

  /////////////////////////////////////////////////////////
  // Print levels histogram
  /////////////////////////////////////////////////////////

  if (print_hist)
  {
    sample_t hist[MAX_HISTOGRAM];
    sample_t max_level;
    int dbpb;
    int i, j;

    dvd_graph.proc.get_output_histogram(hist, MAX_HISTOGRAM);
    max_level = dvd_graph.proc.get_max_level();
    dbpb = dvd_graph.proc.get_dbpb();

    printf("\nHistogram:\n");
    printf("------------------------------------------------------------------------------\n");
    for (i = 0; i*dbpb < 100 && i < MAX_HISTOGRAM; i++)
    {
      printf("%2idB: %4.1f ", i * dbpb, hist[i] * 100);
      for (j = 0; j < 67 && j < hist[i] * 67; j++)
        printf("*");
      printf("\n");
    }
    printf("------------------------------------------------------------------------------\n");
    printf("max_level;%f\ndbpb;%i\nhistogram;", max_level, dbpb);
    for (i = 0; i < MAX_HISTOGRAM; i++)
      printf("%.4f;", hist[i]);
    printf("\n");
    printf("------------------------------------------------------------------------------\n");
  }

  return 0;
}
