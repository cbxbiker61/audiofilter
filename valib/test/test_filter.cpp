/*
  Test filters to comply format change rules and correctness of flushing.


Check buffering
===============

Following buffer processing methods possible:
* In-place. Filter processes data in-place at buffers received with process()
  call. Therefore, upstream must provide buffer that we can safely change. 
  This allows to avoid data copy from buffer to buffer so speed up processing.
  But we must always remember about data references. After process() call
  filter may hold references to upstream data buffers so we must not alter it
  until the end of processing. 
* Immediate. If output data must be larger than input buffer input data is
  processed from input buffer to private output buffer and returned 
  immediately. If filter cannot store all processed data into its output 
  buffer it produces several output chunks. This method does not produce
  processing lag.
* Block buffering. To process data filter requires a block of data that is
  processed at once. So filter get input data until internal buffer fills up 
  and only after this generates output chunk. Also it may hold some amount of
  data in buffer to process next block. This method produces processing lag.
  Also this method requires flushing to release data locked in buffer and 
  correctly finish the stream.

Filters may use different processing models for different input formats. For
example if mixer does inplace processing if output number of channels is less
or equal to input number of channels (faster) and uses immediate buffering 
otherwise.

We can check buffering method used by following conditions:
* Filter that does in-place processing must return pointer to the input buffer.
* If filter does conversion between linear and rawdata it obviously cannot do
  it inplace and uses immediate buffering.
* If filter can process 1 byte/sample and generates output chunk immediately
  it uses immediate buffering. We can find buffer size by increasing input 
  chunk size by one sample/byte until filter to generate 2 output chunks for 1
  input chunk. Buffer size equals to input chunk size - 1.
* If filter remains empty after processing chunk with 1 byte/sample it uses 
  block buffering. We can find buffer size by sending 1 byte/sample chunks 
  until filter to generate output chunk. Buffer size equals to number of input
  chunks but size of output chunk may be less. Difference between buffer size
  and size of output chunk equals to size of data remains at buffer. This
  produces a lag between input and output.
  WRONG. FILTER MAY REDUCE DATA SIZE.

Format change
=============

* Forced format change.
  Buffer size (cycled state, process):
  * very small buffer (5 bytes)
  * very large buffer (32Kb)

  Filter state:
  * filter is in clear state
  * filter is full (only set_input())
  * filter is cycled through full state to empty
  * filter is cycled and full again (only set_input())
  Methods:
  * set_input()
  * process() with empty chunk
  * process() with data chunk
  New format:
  * dummy
  * same format
  * new format
  * unsupported format

  Total: 24 cases
  * set_input(): 12 cases
  * process(empty): 6 cases
  * process(data): 6 cases

* Flushing
  Filter state:
  * filter is in clear state
  * filter is cycled through full state to empty
  Methods:
  * process() with empty eos-chunk
  * process() with data eos-chunk
  New format:
  * same format
  * new format
  * unsupported format

  Total: 12 cases


* OFDD Forced format change
  Filter state:
  * filter is in clear state
  * filter is cycled through full state to empty

  * (ofdd) filter is in pre-stream change format state
  * (ofdd) filter is in transition state (we may not have this state)
  * (ofdd) filter is in empty state after stream change
  * (ofdd) filter is cycled after stream change
  Methods:
  * process() with empty chunk
  * process() with data chunk
  * process() with empty eos-chunk
  * process() with data eos-chunk
  * (ofdd) process() with data chunk with format change (same format only)
  New format:
  * same format
  * new format
  * unsupported format

  we may need to test set_input() only after stream change
*/

#include "log.h"
#include "filter_tester.h"
#include "test_source.h"
#include "all_filters.h"

#include "filter_graph.h"
#include "fir/param_fir.h"

static const int formats[] = 
{ 
  FORMAT_UNKNOWN, // unspecified format
  FORMAT_RAWDATA,
  FORMAT_LINEAR,
  FORMAT_PCM16, FORMAT_PCM24, FORMAT_PCM32,
  FORMAT_PCM16_BE, FORMAT_PCM24_BE, FORMAT_PCM32_BE, 
  FORMAT_PCMFLOAT, FORMAT_PCMDOUBLE,
  FORMAT_PES, FORMAT_SPDIF,
  FORMAT_AC3, FORMAT_MPA, FORMAT_DTS,
  FORMAT_LPCM20, FORMAT_LPCM24
};

static const int modes[] = 
{
  0, // unspecified mode
  MODE_1_0,     MODE_2_0,     MODE_3_0,     MODE_2_1,     MODE_3_1,     MODE_2_2,     MODE_3_2,
  MODE_1_0_LFE, MODE_2_0_LFE, MODE_3_0_LFE, MODE_2_1_LFE, MODE_3_1_LFE, MODE_2_2_LFE, MODE_3_2_LFE
};

static const int sample_rates[] = 
{
  0, // unspecified sample rate
  192000, 96000, 48000, 24000, 12000,
  176400, 88200, 44100, 22050, 11025, 
  128000, 64000, 32000, 16000, 8000,
};

static const int n_formats = array_size(formats);
static const int n_modes = array_size(modes);
static const int n_sample_rates = array_size(sample_rates);




int test_rules(Log *log);
int test_rules_filter(Log *log, Filter *filter, const char *filter_name, 
  Speakers spk_supported1, const char *file_name1, 
  Speakers spk_supported2, const char *file_name2, 
  Speakers spk_unsupported);
int test_rules_filter_int(Log *log, Filter *filter,
  Speakers spk_supported, const char *filename, 
  Speakers spk_supported2, const char *filename2, 
  Speakers spk_unsupported, 
  size_t data_size);

int test_rules(Log *log)
{
  // Base filters
  NullFilter     null(FORMAT_MASK_LINEAR);
  FilterGraph    filter_graph(FORMAT_MASK_LINEAR);

  // Rawdata filters
  Converter      conv_ll(2048);
  Converter      conv_lr(2048);
  Converter      conv_rl(2048);

  AudioDecoder   dec_mpa;
  AudioDecoder   dec_mpa_mix;
  AudioDecoder   dec_ac3;
  AudioDecoder   dec_ac3_mix;
  AudioDecoder   dec_dts;
  AudioDecoder   dec_mix;

  Demux          demux;
  Spdifer        spdifer;
  Despdifer      despdifer;
  Detector       detector;

  // Processing filters
  AGC            agc(2048);
  Mixer          mixer_ip(2048); // inplace
  Mixer          mixer_ib(2048); // immediate
  Resample       resample_up;    // upsample
  Resample       resample_down;  // downsample
  ParamFIR       low_pass_ir(FIR_LOW_PASS, 0.5, 0.0, 0.1, 100, true);
  Convolver      convolver(&low_pass_ir);
  Delay          delay;
  BassRedir      bass_redir_ip; // inplace
  BassRedir      bass_redir_ib; // immediate
  Levels         levels;
  Syncer         dejitter;

  // Aggregate filters
  DVDGraph       dvd;
  DVDGraph       dvd_spdif;

  FilterChain    chain;
  AudioProcessor proc(2048);

  // Setup filters

  conv_ll.set_format(FORMAT_LINEAR);
  conv_lr.set_format(FORMAT_PCM16);
  conv_rl.set_format(FORMAT_LINEAR);
  mixer_ip.set_output(Speakers(FORMAT_LINEAR, MODE_STEREO, 48000));
  mixer_ib.set_output(Speakers(FORMAT_LINEAR, MODE_5_1, 48000));
  resample_up.set_sample_rate(48000);
  resample_down.set_sample_rate(44100);
  bass_redir_ip.set_freq(120);
  bass_redir_ip.set_enabled(true);
  bass_redir_ib.set_freq(120);
  bass_redir_ib.set_enabled(true);
  proc.set_user(Speakers(FORMAT_PCM16, 0, 0));
  dvd_spdif.set_spdif(true, FORMAT_CLASS_SPDIFABLE, false, true, false);

  log->open_group("Test filters");

  // Base filters

  test_rules_filter(log, &null,    "NullFilter", 
    Speakers(FORMAT_LINEAR, MODE_STEREO, 48000), 0,
    Speakers(FORMAT_LINEAR, MODE_5_1, 96000), 0,
    Speakers(FORMAT_RAWDATA, MODE_STEREO, 48000));

  test_rules_filter(log, &filter_graph, "FilterGraph", 
    Speakers(FORMAT_LINEAR, MODE_STEREO, 48000), 0,
    Speakers(FORMAT_LINEAR, MODE_5_1, 96000), 0,
    Speakers(FORMAT_RAWDATA, MODE_STEREO, 48000));

  // Rawdata filters

  test_rules_filter(log, &conv_ll, "Converter linear->linear",
    Speakers(FORMAT_LINEAR, MODE_STEREO, 48000), 0,
    Speakers(FORMAT_LINEAR, MODE_5_1, 96000), 0,
    Speakers(FORMAT_AC3, MODE_STEREO, 48000));

  test_rules_filter(log, &conv_lr, "Converter linear->raw",
    Speakers(FORMAT_LINEAR, MODE_STEREO, 48000), 0,
    Speakers(FORMAT_LINEAR, MODE_5_1, 96000), 0,
    Speakers(FORMAT_AC3, MODE_STEREO, 48000));

  test_rules_filter(log, &conv_rl, "Converter raw->linear",
    Speakers(FORMAT_PCM16, MODE_STEREO, 48000), 0,
    Speakers(FORMAT_PCM32, MODE_5_1, 96000), 0,
    Speakers(FORMAT_AC3, MODE_STEREO, 48000));

  // Rawdata (ofdd) filters

  test_rules_filter(log, &dec_mpa, "AudioDecoder (MPA)",
    Speakers(FORMAT_MPA, MODE_STEREO, 48000), "a.mp2.005.mp2",
    Speakers(FORMAT_RAWDATA, 0, 0), "a.mp2.002.mp2",
    Speakers(FORMAT_LINEAR, MODE_STEREO, 48000));

  test_rules_filter(log, &dec_mpa_mix, "AudioDecoder (MPA mix)",
    Speakers(FORMAT_MPA, 0, 48000), "a.mp2.mix.mp2",
    Speakers(FORMAT_RAWDATA, 0, 0), "a.mp2.002.mp2",
    Speakers(FORMAT_LINEAR, MODE_STEREO, 48000));

  test_rules_filter(log, &dec_ac3, "AudioDecoder (AC3)",
    Speakers(FORMAT_AC3, MODE_STEREO, 48000), "a.ac3.03f.ac3",
    Speakers(FORMAT_RAWDATA, 0, 0), "a.ac3.005.ac3",
    Speakers(FORMAT_LINEAR, MODE_STEREO, 48000));

  test_rules_filter(log, &dec_ac3_mix, "AudioDecoder (AC3 mix)",
    Speakers(FORMAT_AC3, 0, 48000), "a.ac3.mix.ac3",
    Speakers(FORMAT_RAWDATA, 0, 0), "a.ac3.005.ac3",
    Speakers(FORMAT_LINEAR, MODE_STEREO, 48000));

  test_rules_filter(log, &dec_dts, "AudioDecoder (DTS)",
    Speakers(FORMAT_DTS, MODE_5_1, 48000), "a.dts.03f.dts",
    Speakers(FORMAT_RAWDATA, 0, 0), "a.dts.03f.dts",
    Speakers(FORMAT_LINEAR, MODE_STEREO, 48000));

  test_rules_filter(log, &demux, "Demuxer",
    Speakers(FORMAT_PES, 0, 0), "a.madp.mix.pes",
    Speakers(FORMAT_PES, MODE_STEREO, 48000), "a.ac3.03f.pes",
    Speakers(FORMAT_LINEAR, MODE_STEREO, 48000));

  test_rules_filter(log, &dec_mix, "AudioDecoder (mix)",
    Speakers(FORMAT_RAWDATA, 0, 48000), "a.mad.mix.mad",
    Speakers(FORMAT_RAWDATA, 0, 0), "a.mad.mix.mad",
    Speakers(FORMAT_LINEAR, MODE_STEREO, 48000));

  test_rules_filter(log, &spdifer, "Spdifer",
    Speakers(FORMAT_RAWDATA, 0, 0), "a.mad.mix.mad",
    Speakers(FORMAT_AC3, 0, 0), "a.ac3.03f.ac3",
    Speakers(FORMAT_LINEAR, MODE_STEREO, 48000));

  test_rules_filter(log, &despdifer, "Despdifer",
    Speakers(FORMAT_RAWDATA, 0, 0), "a.mad.mix.spdif",
    Speakers(FORMAT_SPDIF, 0, 0), "a.ac3.03f.spdif",
    Speakers(FORMAT_LINEAR, MODE_STEREO, 48000));

  test_rules_filter(log, &detector, "Detector",
    Speakers(FORMAT_RAWDATA, 0, 0), "a.madp.mix.madp",
    Speakers(FORMAT_SPDIF, 0, 0), "a.madp.mix.spdif",
    Speakers(FORMAT_LINEAR, MODE_STEREO, 48000));

  // Linear format processing filters

  test_rules_filter(log, &agc, "AGC",
    Speakers(FORMAT_LINEAR, MODE_STEREO, 48000), 0,
    Speakers(FORMAT_LINEAR, MODE_5_1, 96000), 0,
    Speakers(FORMAT_AC3, MODE_STEREO, 48000));

  test_rules_filter(log, &mixer_ip, "Mixer (inplace)",
    Speakers(FORMAT_LINEAR, MODE_STEREO, 48000), 0,
    Speakers(FORMAT_LINEAR, MODE_5_1, 96000), 0,
    Speakers(FORMAT_AC3, MODE_STEREO, 48000));
    
  test_rules_filter(log, &mixer_ib, "Mixer (immediate)",
    Speakers(FORMAT_LINEAR, MODE_STEREO, 48000), 0,
    Speakers(FORMAT_LINEAR, MODE_5_1, 96000), 0,
    Speakers(FORMAT_AC3, MODE_STEREO, 48000));

  test_rules_filter(log, &resample_up, "Resample (upsample)",
    Speakers(FORMAT_LINEAR, MODE_STEREO, 44100), 0,
    Speakers(FORMAT_LINEAR, MODE_5_1, 44100), 0,
    Speakers(FORMAT_AC3, MODE_STEREO, 48000));

  test_rules_filter(log, &resample_down, "Resample (downsmple)",
    Speakers(FORMAT_LINEAR, MODE_STEREO, 48000), 0,
    Speakers(FORMAT_LINEAR, MODE_5_1, 48000), 0,
    Speakers(FORMAT_AC3, MODE_STEREO, 48000));

  test_rules_filter(log, &convolver, "Convolver",
    Speakers(FORMAT_LINEAR, MODE_STEREO, 48000), 0,
    Speakers(FORMAT_LINEAR, MODE_5_1, 48000), 0,
    Speakers(FORMAT_AC3, MODE_STEREO, 48000));

  test_rules_filter(log, &delay, "Delay",
    Speakers(FORMAT_LINEAR, MODE_STEREO, 48000), 0,
    Speakers(FORMAT_LINEAR, MODE_5_1, 96000), 0,
    Speakers(FORMAT_AC3, MODE_STEREO, 48000));

  test_rules_filter(log, &bass_redir_ip, "BassRedir (inplace)",
    Speakers(FORMAT_LINEAR, MODE_5_1, 48000), 0,
    Speakers(FORMAT_LINEAR, MODE_STEREO, 96000), 0,
    Speakers(FORMAT_AC3, MODE_STEREO, 48000));
    
  test_rules_filter(log, &bass_redir_ib, "BassRedir (immediate)",
    Speakers(FORMAT_LINEAR, MODE_STEREO, 48000), 0,
    Speakers(FORMAT_LINEAR, MODE_5_1, 96000), 0,
    Speakers(FORMAT_AC3, MODE_STEREO, 48000));
    
  test_rules_filter(log, &levels, "Levels",
    Speakers(FORMAT_LINEAR, MODE_STEREO, 48000), 0,
    Speakers(FORMAT_LINEAR, MODE_5_1, 96000), 0,
    Speakers(FORMAT_AC3, MODE_STEREO, 48000));

  test_rules_filter(log, &dejitter, "Dejitter",
    Speakers(FORMAT_LINEAR, MODE_STEREO, 48000), 0,
    Speakers(FORMAT_LINEAR, MODE_5_1, 96000), 0,
    Speakers(FORMAT_AC3, MODE_STEREO, 48000));

  // Aggregate filters

  test_rules_filter(log, &proc, "Processor",
    Speakers(FORMAT_PCM16, MODE_STEREO, 48000), 0,
    Speakers(FORMAT_LINEAR, MODE_5_1, 96000), 0,
    Speakers(FORMAT_AC3, MODE_STEREO, 48000));

/*
  test_rules_filter(log, &dvd, "DVDGraph",
    Speakers(FORMAT_PES, 0, 0), "a.madp.mix.pes",
    Speakers(FORMAT_AC3, 0, 0), "a.ac3.mix.ac3",
    Speakers(FORMAT_OGG, MODE_STEREO, 48000));

  test_rules_filter(log, &dvd_spdif, "DVDGraph (spdif)",
    Speakers(FORMAT_PES, 0, 0), "a.madp.mix.pes",
    Speakers(FORMAT_AC3, 0, 0), "a.ac3.mix.ac3",
    Speakers(FORMAT_OGG, MODE_STEREO, 48000));
*/
  return log->close_group();
}



int test_rules_filter(Log *log, Filter *filter, const char *filter_name, 
  Speakers spk_supported, const char *filename, 
  Speakers spk_supported2, const char *filename2, 
  Speakers spk_unsupported)
/*
This test ensures that filter complies all format change rules.

Processing is done on 2 chunk sizes - large and small chunks because some 
format change problems appears only with small or only with large chunks.

To do the test we need data sources. Test uses file source if filename was
specified and noise source otherwise.

We need 2 data sources to verify correctness of switching between streams.
*/
{
  const size_t small_data_size = 5;
  const size_t large_data_size = 32768;

  FilterTester f(filter, log);

  log->open_group("Testing %s", filter_name);

  /////////////////////////////////////////////////////////
  // Filter should be created in uninitialized state

  if (f.get_input() != spk_unknown)
    log->msg("Filter was created in initialized state");

  /////////////////////////////////////////////////////////
  // Check output format dependency

  if (!f.set_input(spk_supported))
  {
    log->err("Set format: %s %s %i failed", 
      spk_supported.format_text(), spk_supported.mode_text(), spk_supported.sample_rate);
    return log->close_group();
  }

  if (f.is_ofdd())
    log->msg("Output format is data-dependent");

  /////////////////////////////////////////////////////////
  // Format change crash test.
  // Try to set numerous formats. 

  log->msg("Format change crash test");
  for (int i_format = 0; i_format < n_formats; i_format++)
    for (int i_mode = 0; i_mode < n_modes; i_mode++)
      for (int i_sample_rate = 0; i_sample_rate < n_sample_rates; i_sample_rate++)
        f.set_input(Speakers(formats[i_format], modes[i_mode], sample_rates[i_sample_rate]));

  /////////////////////////////////////////////////////////
  // Main tests

  log->msg("Small buffer (%i)", small_data_size);
  test_rules_filter_int(log, filter, 
    spk_supported, filename, 
    spk_supported2, filename2, 
    spk_unsupported, small_data_size);

  log->msg("Large buffer (%i)", large_data_size);
  test_rules_filter_int(log, filter, 
    spk_supported, filename, 
    spk_supported2, filename2, 
    spk_unsupported, large_data_size);

  return log->close_group();
}


int test_rules_filter_int(Log *log, Filter *filter,
  Speakers spk_supported, const char *filename, 
  Speakers spk_supported2, const char *filename2, 
  Speakers spk_unsupported, 
  size_t data_size)
{
  TestSource src;
  Chunk chunk;

  FilterTester f(filter, log);

  /////////////////////////////////////////////////////////
  // Most used operations 

  // filter operations with error-checking

  #define SET_INPUT_OK(spk, err_text)   \
    if (!f.set_input(spk))              \
      return log->err(err_text, spk.format_text(), spk.mode_text(), spk.sample_rate);

  #define SET_INPUT_FAIL(spk, err_text) \
    if (f.set_input(spk))               \
      return log->err(err_text, spk.format_text(), spk.mode_text(), spk.sample_rate);

  #define PROCESS_OK(chunk, err_text)   \
    if (!f.process(&chunk))             \
      return log->err(err_text, chunk.spk.format_text(), chunk.spk.mode_text(), chunk.spk.sample_rate);

  #define PROCESS_FAIL(chunk, err_text) \
    if (f.process(&chunk))              \
      return log->err(err_text, chunk.spk.format_text(), chunk.spk.mode_text(), chunk.spk.sample_rate);

  #define GET_CHUNK_OK(chunk, err_text) \
    if (!f.get_chunk(&chunk))           \
      return log->err(err_text);

  // batch operations

  #define FILL_FILTER                   \
    while (f.is_empty())                \
    {                                   \
      if (src.is_empty())               \
        return log->err("Cannot fill a filter"); \
      src.get_chunk(&chunk);            \
      PROCESS_OK(chunk, "process(%s %s %i) failed"); \
    }

  #define EMPTY_FILTER                  \
    while (!f.is_empty())               \
      GET_CHUNK_OK(chunk, "get_chunk() failed");

  // init filter to several standard test states

  #define INIT_EMPTY(spk)               \
    SET_INPUT_OK(spk_supported, "Set format: %s %s %i failed");

  #define INIT_FULL(spk, filename)      \
    INIT_EMPTY(spk);                    \
    src.open(spk, filename, data_size); \
    FILL_FILTER;

  #define INIT_CYCLED(spk, filename)    \
    INIT_EMPTY(spk);                    \
    src.open(spk, filename, data_size); \
    FILL_FILTER;                        \
    EMPTY_FILTER;

  // post-cycle to ensure correct filter state

  #define POST_CYCLE                    \
    FILL_FILTER;                        \
    EMPTY_FILTER;

  #define POST_NEW_CYCLE(spk, filename) \
    src.open(spk, filename, data_size); \
    FILL_FILTER;                        \
    EMPTY_FILTER;

  /////////////////////////////////////////////////////////
  // Check sources
  // File must be larger than 3 * data_size 
  // (pre-cycle, process, post-cycle).

  src.open(spk_supported, filename, 3*data_size);
  if (!src.is_open())
    return log->err("Cannot open file %s", filename);

  if (src.is_empty())
    return log->err("Source is empty");

  if (!src.get_chunk(&chunk))
    return log->err("Cannot get chunk from source");

  if (chunk.spk != spk_supported)
    return log->err("Source has wrong fromat");

  if (chunk.size != 3*data_size)
    return log->err("Source has wrong chunk size");

  src.open(spk_supported2, filename2, 3*data_size);
  if (!src.is_open())
    return log->err("Cannot open file %s", filename2);

  if (src.is_empty())
    return log->err("Source2 is empty");

  if (!src.get_chunk(&chunk))
    return log->err("Cannot get chunk from source2");

  if (chunk.spk != spk_supported2)
    return log->err("Source2 has wrong format");

  if (chunk.size != 3*data_size)
    return log->err("Source2 has wrong chunk size");

  /////////////////////////////////////////////////////////
  // Determine buffering

  {
    size_t size = 0;
    bool empty_chunks = false;

    src.open(spk_supported, filename, 1);
    src.get_chunk(&chunk);
    SET_INPUT_OK(spk_supported, "Set format: %s %s %i failed");
    PROCESS_OK(chunk, "process(%s %s %i) failed");

    // block-buffering filter may generate empty chunks
    // but it is bad strategy
    while (!f.is_empty())
    {
      GET_CHUNK_OK(chunk, "get_chunk() failed");
      size += chunk.size;
      if (chunk.is_empty())
        empty_chunks = true;
    }

    if (size == 0)
      log->msg("Filter possibly uses block buffering");

    if (empty_chunks)
      log->msg("Filter generates unnesesary empty chunks");
  }

  /////////////////////////////////////////////////////////
  // Forced format change
  //
  // Test format change scenarios. Most of work is done by 
  // FilterTester so we do not explicitly check a filter
  // to actually change the stream. We just run different
  // scenarios to force our traps to work...
  // 
  // Tests 1-4 do not use post cycle because elements of 
  // this cycle are not tested yet. Tests 5-7 use post 
  // cycle to ensure filter state correctness.
  //
  // List of tests:                 format: same new wrong dummy
  // 1. Empty filter, set_input()            +    +    +     -
  // 2. Empty filter, process(empty chunk)   +    +    +     +
  // 3. Empty filter, process(data chunk)    +    +    +     +
  // 4. Full filter, set_input()             +    +    +     -
  // 5. Cycled filter, set_input()           +    +    +     -
  // 6. Cycled filter, process(empty chunk)  -    +    +     +
  // 7. Cycled filter, process(data chunk)   -    +    +     +
  // 8. Cycled full filter, set_input()      +    +    +     -
  //
  // Total scenarios: 26
  /////////////////////////////////////////////////////////


  /////////////////////////////////////////////////////////
  // Forced format change 1. 
  // Empty filter, set_input()

  log->msg("Forced format change 1. Empty filter, set_input()");

  // 1.1 - format change to the same format
  INIT_EMPTY(spk_supported);
  SET_INPUT_OK(spk_supported,     "set_input(same format: %s %s %i) failed");

  // 1.2 - format change to the new format
  INIT_EMPTY(spk_supported);
  SET_INPUT_OK(spk_supported2,    "set_input(new format: %s %s %i) failed");

  // 1.3 - format change to the unsupported format
  INIT_EMPTY(spk_supported);
  SET_INPUT_FAIL(spk_unsupported, "set_input(wrong format: %s %s %i) succeeded");

  /////////////////////////////////////////////////////////
  // Forced format change 2. 
  // Empty filter, process(empty chunk)

  log->msg("Forced format change 2. Empty filter, process(empty chunk)");

  // 2.1 - format change to the same format
  INIT_EMPTY(spk_supported);
  chunk.set_empty(spk_supported);
  PROCESS_OK(chunk,               "process(same format: %s %s %i) failed");

  // 2.2 - format change to the new format
  INIT_EMPTY(spk_supported);
  chunk.set_empty(spk_supported2);
  PROCESS_OK(chunk,               "process(new format: %s %s %i) failed");

  // 2.3 - format change to the unsupported format
  INIT_EMPTY(spk_supported);
  chunk.set_empty(spk_unsupported);
  PROCESS_FAIL(chunk,             "process(wrong format: %s %s %i) succeeded");

  // 2.4 - dummy processing
  INIT_EMPTY(spk_supported);
  chunk.set_dummy();
  PROCESS_OK(chunk,               "process(dummy) failed");

  /////////////////////////////////////////////////////////
  // Forced format change 3. 
  // Empty filter, process(data chunk)

  log->msg("Forced format change 3. Empty filter, process(data chunk)");

  // 3.1 - format change to the same format
  INIT_EMPTY(spk_supported);
  src.open(spk_supported, filename, data_size);
  src.get_chunk(&chunk);
  PROCESS_OK(chunk,               "process(same format: %s %s %i) failed");

  // 3.2 - format change to the new format
  INIT_EMPTY(spk_supported);
  src.open(spk_supported2, filename2, data_size);
  src.get_chunk(&chunk);
  PROCESS_OK(chunk,               "process(new format: %s %s %i) failed");

  // 3.3 - format change to the unsupported format
  // (use noise source for unsupported format)
  INIT_EMPTY(spk_supported);
  src.open(spk_unsupported, 0, data_size);
  src.get_chunk(&chunk);
  PROCESS_FAIL(chunk,             "process(wrong format: %s %s %i) succeeded");

  // 3.4 - dummy processing
  // (use noise source for unknown format)
  INIT_EMPTY(spk_supported);
  src.open(spk_unknown, 0, data_size);
  src.get_chunk(&chunk);
  PROCESS_OK(chunk,               "process(dummy) failed");

  /////////////////////////////////////////////////////////
  // Forced format change 4. 
  // Full filter, set_input()

  log->msg("Forced format change 4. Full filter, set_input()");

  // 4.1 - format change to the same format
  INIT_FULL(spk_supported, filename);
  SET_INPUT_OK(spk_supported,     "set_input(same format: %s %s %i) failed");

  // 4.2 - format change to the new format
  INIT_FULL(spk_supported, filename);
  SET_INPUT_OK(spk_supported2,    "set_input(new format: %s %s %i) failed");

  // 4.3 - format change to the usupported format
  INIT_FULL(spk_supported, filename);
  SET_INPUT_FAIL(spk_unsupported, "set_input(wrong format: %s %s %i) succeeded");

  /////////////////////////////////////////////////////////
  // Forced format change 5. 
  // Cycled filter, set_input()

  log->msg("Forced format change 5. Cycled filter, set_input()");

  // 5.1 - format change to the same format
  INIT_CYCLED(spk_supported, filename);
  SET_INPUT_OK(spk_supported,     "set_input(same format: %s %s %i) failed");
  POST_NEW_CYCLE(spk_supported, filename);

  // 5.2 - format change to the new format
  INIT_CYCLED(spk_supported, filename);
  SET_INPUT_OK(spk_supported2,    "set_input(new format: %s %s %i) failed");
  POST_NEW_CYCLE(spk_supported2, filename2);

  // 5.3 - format change to the unsupported format
  INIT_CYCLED(spk_supported, filename);
  SET_INPUT_FAIL(spk_unsupported, "set_input(wrong format: %s %s %i) succeeded");
  POST_NEW_CYCLE(spk_supported, filename);

  /////////////////////////////////////////////////////////
  // Forced format change 6.
  // Cycled filter, process(empty chunk)

  log->msg("Forced format change 6. Cycled filter, process(empty chunk)");

  // 6.1 - format change to the new format
  INIT_CYCLED(spk_supported, filename);
  chunk.set_empty(spk_supported2);
  PROCESS_OK(chunk,               "process(new format: %s %s %i) failed");
  POST_NEW_CYCLE(spk_supported2, filename2);

  // 6.2 - format change to the unsupported format
  INIT_CYCLED(spk_supported, filename);
  chunk.set_empty(spk_unsupported);
  PROCESS_FAIL(chunk,             "process(wrong format: %s %s %i) succeeded");
  POST_NEW_CYCLE(spk_supported, filename);

  // 6.3 - dummy processing
  INIT_CYCLED(spk_supported, filename);
  chunk.set_dummy();
  PROCESS_OK(chunk,               "process(dummy) failed");
  POST_CYCLE(spk_supported2, filename2);

  /////////////////////////////////////////////////////////
  // Forced format change 7.
  // Cycled filter, process(data chunk)

  log->msg("Forced format change 7. Cycled filter, process(data chunk)");

  // 7.1 - format change to the new format
  INIT_CYCLED(spk_supported, filename);
  src.open(spk_supported2, filename2, data_size);
  src.get_chunk(&chunk);
  PROCESS_OK(chunk,               "process(new format: %s %s %i) failed");
  POST_CYCLE;

  // 7.2 - format change to the unsupported format
  // (use noise source for unsupported format)
  INIT_CYCLED(spk_supported, filename);
  src.open(spk_unsupported, 0, data_size);
  src.get_chunk(&chunk);
  PROCESS_FAIL(chunk,             "process(wrong format: %s %s %i) succeeded");
  POST_NEW_CYCLE(spk_supported, filename);

  // 7.3 - dummy processing
  // (use noise source for unknown format)
  INIT_CYCLED(spk_supported, filename);
  {
    NoiseGen noise(spk_unknown, 1, data_size, data_size);
    noise.get_chunk(&chunk);
    PROCESS_OK(chunk,               "process(dummy) failed");
  }
  POST_CYCLE;

  /////////////////////////////////////////////////////////
  // Forced format change 8. 
  // Cycled full filter, set_input()

  log->msg("Forced format change 8. Cycled full filter, set_input()");

  // 8.1 - format change to the same format
  INIT_CYCLED(spk_supported, filename);
  FILL_FILTER;
  SET_INPUT_OK(spk_supported,     "set_input(same format: %s %s %i) failed");
  POST_NEW_CYCLE(spk_supported, filename);

  // 8.2 - format change to the new format
  INIT_CYCLED(spk_supported, filename);
  FILL_FILTER;
  SET_INPUT_OK(spk_supported2,    "set_input(new format: %s %s %i) failed");
  POST_NEW_CYCLE(spk_supported2, filename2);

  // 8.3 - format change to the unsupported format
  INIT_CYCLED(spk_supported, filename);
  FILL_FILTER;
  SET_INPUT_FAIL(spk_unsupported, "set_input(wrong format: %s %s %i) succeeded");
  POST_NEW_CYCLE(spk_supported, filename);

  /////////////////////////////////////////////////////////
  // Flushing
  //
  // Test flushing scenarios. Most of work is done by 
  // FilterTester so we do not explicitly check a filter
  // to end the stream.  We just run different scenarios to
  // force traps to work...
  //
  // List of tests:                 format: same new wrong
  // 1. Empty filter, process(empty chunk)   +    +    +
  // 2. Empty filter, process(data chunk)    +    +    +
  // 3. Cycled filter, process(empty chunk)  +    +    +
  // 4. Cycled filter, process(data chunk)   +    +    +
  //
  // ofdd filter only:
  // 5. Empty filter, process(data with fc)  +    +    +
  // 6. Cycled filter, process(data with fc) +    +    +
  //
  // Total scenarios: 12
  /////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////
  // Flushing 1. 
  // Empty filter, process(empty chunk)

  log->msg("Flushing 1. Empty filter, process(empty chunk)");

  // 1.1 same format (flush current stream)
  INIT_EMPTY(spk_supported);
  chunk.set_empty(spk_supported, false, 0, true);
  PROCESS_OK(chunk,               "process(%s %s %i) failed");
  if (!f.is_empty())
  {
    size_t size = 0;
    while (!f.is_empty())
    {
      GET_CHUNK_OK(chunk,         "get_chunk() failed");
      size += chunk.size;
    }
    if (size)
      log->err("Empty filter generates output on flushing");
  }
//  else
//    log->msg("Empty filter does not pass eos-chunk");

  // 1.2 new format (forced format change and then flush new stream)
  INIT_EMPTY(spk_supported);
  chunk.set_empty(spk_supported2, false, 0, true);
  PROCESS_OK(chunk,               "process(%s %s %i) failed");
  if (!f.is_empty())
  {
    size_t size = 0;
    while (!f.is_empty())
    {
      GET_CHUNK_OK(chunk,         "get_chunk() failed");
      size += chunk.size;
    }
    if (size)
      log->err("Empty filter generates output on flushing");
  }
//  else
//    log->msg("Empty filter does not pass eos-chunk");

  // 1.3 unsupported format (terminate current stream)
  INIT_EMPTY(spk_supported);
  chunk.set_empty(spk_unsupported, false, 0, true);
  PROCESS_FAIL(chunk,             "process(%s %s %i) succeeded");

  /////////////////////////////////////////////////////////
  // Flushing 2. 
  // Empty filter, process(data chunk)

  log->msg("Flushing 2. Empty filter, process(data chunk)");

  // 2.1 same format
  INIT_EMPTY(spk_supported);
  src.open(spk_supported, filename, data_size);
  src.get_chunk(&chunk);
  chunk.eos = true;
  PROCESS_OK(chunk,               "process(%s %s %i) failed");
//  if (f.is_empty())
//    log->msg("Filter does not pass eos-chunk");
//  else
    EMPTY_FILTER;
 
  // 2.2 new format (forced format change and then flush new stream)
  INIT_EMPTY(spk_supported);
  src.open(spk_supported2, filename2, data_size);
  src.get_chunk(&chunk);
  chunk.eos = true;
  PROCESS_OK(chunk,               "process(%s %s %i) failed");
//  if (f.is_empty())
//    log->msg("Filter does not pass eos-chunk");
//  else
    EMPTY_FILTER;

  // 2.3 unsupported format
  INIT_EMPTY(spk_supported);
  src.open(spk_unsupported, 0, data_size);
  src.get_chunk(&chunk);
  chunk.eos = true;
  PROCESS_FAIL(chunk,             "process(%s %s %i) succeeded");

  /////////////////////////////////////////////////////////
  // Flushing 3. 
  // Cycled filter, process(empty chunk)

  log->msg("Flushing 3. Cycled filter, process(empty chunk)");

  // 3.1 same format (flush current stream)
  INIT_CYCLED(spk_supported, filename);
  chunk.set_empty(spk_supported, false, 0, true);
  PROCESS_OK(chunk,               "process(%s %s %i) failed");
  // filter cannot be empty here (check is done by FilterTester)
  EMPTY_FILTER;
 
  // 3.2 new format (forced format change and then flush new stream)
  INIT_CYCLED(spk_supported, filename);
  chunk.set_empty(spk_supported2, false, 0, true);
  PROCESS_OK(chunk,               "process(%s %s %i) failed");
//  if (f.is_empty())
//    log->msg("Filter does not pass eos-chunk");
//  else
    EMPTY_FILTER;

  // 3.3 unsupported format
  INIT_CYCLED(spk_supported, filename);
  chunk.set_empty(spk_unsupported, false, 0, true);
  PROCESS_FAIL(chunk,             "process(%s %s %i) succeeded");
  
  /////////////////////////////////////////////////////////
  // Flushing 4. 
  // Cycled filter, process(data chunk)

  log->msg("Flushing 4. Cycled filter, process(data chunk)");

  // 4.1 same format (just flush the stream)
  INIT_CYCLED(spk_supported, filename);
  src.get_chunk(&chunk);
  chunk.eos = true;
  PROCESS_OK(chunk,               "process(%s %s %i) failed");
  // filter cannot be empty here (check is done by FilterTester)
  EMPTY_FILTER;
 
  // 4.2 new format (forced format change and then flush new stream)
  INIT_CYCLED(spk_supported, filename);
  src.open(spk_supported2, filename2, data_size);
  src.get_chunk(&chunk);
  chunk.eos = true;
  PROCESS_OK(chunk,               "process(%s %s %i) failed");
//  if (f.is_empty())
//    log->msg("Filter does not pass eos-chunk");
//  else
    EMPTY_FILTER;

  // 4.3 unsupported format
  INIT_CYCLED(spk_supported, filename);
  src.open(spk_unsupported, 0, data_size);
  src.get_chunk(&chunk);
  chunk.eos = true;
  PROCESS_FAIL(chunk,             "process(%s %s %i) succeeded");

  /////////////////////////////////////////////////////////
  // Full processing cycle test. 

  log->msg("Full processing cycle test");

  f.reset_streams();
  INIT_EMPTY(spk_supported);
  src.open(spk_supported, filename, data_size);
  while (!src.is_empty())
  {
    FILL_FILTER;
    EMPTY_FILTER;
  }

  int streams1 = f.get_streams();
  if (streams1 < 1)
    log->msg("Stream was not finished");
  else if (streams1 > 1)
    log->msg("Number of streams: %i", streams1);

  /////////////////////////////////////////////////////////
  // All at once processing

  if (filename)
  {
    log->msg("All-at-once processing test");

    // Load the whole file
    MemFile file(filename);
    if (!file)
      return log->err("Cannot load the file %s", filename);
    chunk.set_rawdata(spk_supported, file, file.size());
    chunk.set_eos();

    f.reset_streams();
    PROCESS_OK(chunk,               "process(%s %s %i) failed");
    EMPTY_FILTER;

    int streams2 = f.get_streams();
    if (streams2 < 1)
      log->msg("Stream was not finished");
    else if (streams1 != streams2)
      log->err("Number of streams differ. Full processing: %i. All-at-once: %i", 
        streams1, streams2);
  }


  return 0;
}
