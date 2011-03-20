/*
  DVDGraph filter test
  * file transform test with format changes
  * dynamical user format change
  * dynamical graph rebuild
  * format agreement procedure
*/

#include "suite.h"
#include "filter_tester.h"
#include "filters\dvd_graph.h"
#include <source\raw_source.h>
#include <win32\cpu.h>

///////////////////////////////////////////////////////////////////////////////
// Test constants

static const vtime_t time_per_test = 1.0;    // 1 sec for each speed test
static const size_t min_data_size = 2048*12; // minimum data size to generate after state change:
                                             // size of 6-channel pcm16 data with more than maximum number of samples per frame

class SinkAcceptSpdif : public NullSink
{
public:
  SinkAcceptSpdif() {};
};

class SinkRefuseSpdif : public NullSink
{
public:
  SinkRefuseSpdif() {};

  bool query_input(Speakers _spk) const
  { return _spk.format != FORMAT_SPDIF; }
};

class SinkAllow : public NullSink
{
protected:
  Speakers allow;

public:
  SinkAllow(Speakers _allow)
  { allow = _allow; }

  bool query_input(Speakers _spk) const
  {
    // Check correctness of the proposed format

    if (_spk.format == FORMAT_UNKNOWN)
      return false;

    if (!_spk.sample_rate)
      return false;

    if (!_spk.mask)
      return false;

    // Check format compatibility

    if (allow.format != FORMAT_UNKNOWN && _spk.format != allow.format)
      return false;

    if (allow.mask && _spk.mask != allow.mask)
      return false;

    if (allow.sample_rate && _spk.sample_rate != allow.sample_rate)
      return false;

    return true;
  }
};

///////////////////////////////////////////////////////////////////////////////
// Test class

class DVDGraph_test
{
protected:
  Filter *f;
  FilterTester t;
  DVDGraph dvd;
  Log *log;

public:
  DVDGraph_test(Log *_log)
  {
    log = _log;
    t.link(&dvd, log);
    f = &dvd; // do not use FilterTester
  }

  int test()
  {
    log->open_group("DVDGraph test");
    transform();
    format_agreement();
    user_format_change();
    spdif_rebuild();
    return log->close_group();
  }

  /////////////////////////////////////////////////////////////////////////////
  // Transform test

  void transform()
  {
    log->open_group("Transform test");

    // PES to SPDIF transform with format changes
    dvd.set_sink(0);
    dvd.set_user(Speakers(FORMAT_PCM16, 0, 0));

    log->open_group("Test PES->SPDIF transform");
    dvd.set_spdif(true, FORMAT_CLASS_SPDIFABLE, false, true, true);
    compare_file(log, Speakers(FORMAT_PES, 0, 0), "a.mad.mix.pes",  f, "a.mad.mix.spdif");
    compare_file(log, Speakers(FORMAT_PES, 0, 0), "a.madp.mix.pes", f, "a.madp.mix.spdif");
    log->close_group();

    log->open_group("Test PES->SPDIF transform with SPDIF as PCM output");
    dvd.set_spdif(true, FORMAT_CLASS_SPDIFABLE, true, true, true);
    compare_file(log, Speakers(FORMAT_PES, 0, 0), "a.mad.mix.pes",  f, "a.mad.mix.spdif");
    compare_file(log, Speakers(FORMAT_PES, 0, 0), "a.madp.mix.pes", f, "a.madp.mix.spdif");
    log->close_group();

    log->open_group("Test SPDIF->SPDIF transform");
    dvd.set_spdif(true, FORMAT_CLASS_SPDIFABLE, false, true, true);
    compare_file(log, Speakers(FORMAT_SPDIF, 0, 0), "a.mad.mix.spdif",  f, "a.mad.mix.spdif");
    log->close_group();

    log->open_group("Test PCM->SPDIF transform (using detector)");
    dvd.set_use_detector(true);
    dvd.set_spdif(true, FORMAT_CLASS_SPDIFABLE, false, true, true);
    compare_file(log, Speakers(FORMAT_PCM16, MODE_STEREO, 48000), "a.madp.mix.spdif",  f, "a.madp.mix.spdif");
    log->close_group();

    log->close_group();
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
      Speakers(FORMAT_PCM16, 0, 0),            // pcm16 as-is
      Speakers(FORMAT_PCM16, MODE_STEREO, 0),  // pcm16 stereo (possible downmix)
      Speakers(FORMAT_PCM16, MODE_5_1, 0),     // pcm16 5.1 (possible upmix)
      Speakers(FORMAT_PCM32, 0, 0),            // pcm32 as-is
      Speakers(FORMAT_PCM32, MODE_STEREO, 0),  // pcm32 stereo (possible downmix)
      Speakers(FORMAT_PCM32, MODE_5_1, 0),     // pcm32 5.1 (possible upmix)
      // spdif pt
      Speakers(FORMAT_PCM16, 0, 0),            // pcm16 as-is
      Speakers(FORMAT_PCM16, MODE_STEREO, 0),  // pcm16 stereo (possible downmix)
      Speakers(FORMAT_PCM16, MODE_5_1, 0),     // pcm16 5.1 (possible upmix)
      Speakers(FORMAT_PCM32, 0, 0),            // pcm32 as-is
      Speakers(FORMAT_PCM32, MODE_STEREO, 0),  // pcm32 stereo (possible downmix)
      Speakers(FORMAT_PCM32, MODE_5_1, 0),     // pcm32 5.1 (possible upmix)
    };
    bool spdif_stereo_pt[] = {
      false, false, false, false, false, false, 
      true,  true,  true,  true,  true,  true,
    };

    log->open_group("Testing %s (%s %s %i)", file_name, 
      spk.format_text(), spk.mode_text(), spk.sample_rate);

    Chunk chunk;
    RAWSource src(spk, file_name, 2048);

    if (!src.is_open())
      return log->err_close("Cannot open file %s", file_name);

    if (!f->set_input(spk))
      return log->err_close("dvd.set_input(%s %s %iHz) failed", 
        spk.format_text(), spk.mode_text(), spk.sample_rate);

    for (int i = 0; i < array_size(formats); i++)
    {
      dvd.set_user(formats[i]);
      dvd.set_spdif(spdif_stereo_pt[i], 0, false, true, spdif_stereo_pt[i]);

      Speakers test_spk = spk;
      if (formats[i].format)
      {
        test_spk.format = formats[i].format;
        test_spk.level = formats[i].level;
      }

      if (formats[i].mask)
        test_spk.mask = formats[i].mask;

      if (spdif_stereo_pt[i] && test_spk.mask != MODE_STEREO)
      {
        test_spk.format = FORMAT_SPDIF;
        test_spk.level = 1.0;
      }

      while (!src.is_empty() && f->get_output() != test_spk)
        if (f->is_empty())
        {
          if (!src.get_chunk(&chunk))
            return log->err_close("src->get_chunk() failed");

          if (!f->process(&chunk))
            return log->err_close("dvd.process() failed");
        }
        else
        {
          if (!f->get_chunk(&chunk))
            return log->err_close("dvd.get_chunk() failed");
        }

      if (f->get_output() != test_spk)
        return log->err_close("cannot change format: %s%s %s -> %s %s",
          spdif_stereo_pt[i]? "(SPDIF/stereo passthrough) ": "",
          formats[i].format? formats[i].format_text(): "as-is",
          formats[i].mask? formats[i].mode_text(): "as-is",
          test_spk.format_text(), test_spk.mode_text());
      else
        log->msg("successful format change: %s%s %s -> %s %s",
          spdif_stereo_pt[i]? "(SPDIF/stereo passthrough) ": "",
          formats[i].format? formats[i].format_text(): "as-is",
          formats[i].mask? formats[i].mode_text(): "as-is",
          test_spk.format_text(), test_spk.mode_text());
    }

    return log->close_group();
  }

  /////////////////////////////////////////////////////////////////////////////
  // Test transitions between main SPDIF modes
  // (decode, passthrough, encode, stereo passthrough)

  void spdif_rebuild()
  {
    /////////////////////////////////////////////////////////
    // Dynamical graph rebuild
    log->open_group("Dynamical graph rebuild");

    // Raw streams
    spdif_rebuild("a.ac3.03f.ac3", Speakers(FORMAT_AC3, 0, 0), true, true);
    spdif_rebuild("a.dts.03f.dts", Speakers(FORMAT_DTS, 0, 0), true, true);
    spdif_rebuild("a.mp2.005.mp2", Speakers(FORMAT_MPA, 0, 0), true, true);

    // PES streams
    spdif_rebuild("a.ac3.03f.pes", Speakers(FORMAT_PES, 0, 0), true, true);
    spdif_rebuild("a.dts.03f.pes", Speakers(FORMAT_PES, 0, 0), true, true);
    spdif_rebuild("a.mp2.005.pes", Speakers(FORMAT_PES, 0, 0), true, true);

    // PCM streams
    spdif_rebuild("a.pcm.005.lpcm", Speakers(FORMAT_PCM16_BE, MODE_STEREO, 48000), false, true);
    spdif_rebuild("a.pcm.005.lpcm", Speakers(FORMAT_PCM16_BE, MODE_STEREO, 45000), false, false);
    spdif_rebuild("a.pcm.005.pes",  Speakers(FORMAT_PES, 0, 0), false, true);

    log->close_group();
  }

  int spdif_rebuild(const char *file_name, Speakers spk, bool is_spdifable, bool can_encode)
  {
    log->open_group("Testing %s (%s)%s%s", 
      file_name, spk.format_text(), 
      is_spdifable? "": " (not spdifable)",
      can_encode? "": " (cannot encode)");

    RAWSource src(spk, file_name, 2048);
    if (!src.is_open())
      return log->err_close("Cannot open file %s", file_name);

    if (!f->set_input(spk))
      return log->err_close("dvd.set_input(%s %s %iHz) failed", 
        spk.format_text(), spk.mode_text(), spk.sample_rate);

    // Check all sink modes
    // (no sink, spdif allowed, spdif refused)

    SinkAcceptSpdif sink_accept_spdif;
    SinkRefuseSpdif sink_refuse_spdif;
    const Sink *sink[] = { 0, &sink_accept_spdif, &sink_refuse_spdif };
    const bool spdif_allowed[] = { true, true, false };
    const char *sink_name[] = { "no sink", "sink that accepts spdif", "sink that refuses spdif" };

    for (int isink = 0; isink < array_size(sink); isink++)
    {
      log->msg("Test with %s", sink_name[isink]);
      dvd.set_sink(sink[isink]);

      // Check all possible transition between spdif modes
      // (decode, passthrough, encode, stereo passthrough)

      if (is_spdifable)
      {
        test_decode(&src);
        test_passthrough(&src, spdif_allowed[isink]);
        test_encode(&src, spdif_allowed[isink], can_encode);
        test_stereo_passthrough(&src);

        test_decode(&src);
        test_encode(&src, spdif_allowed[isink], can_encode);
        test_decode(&src);

        test_stereo_passthrough(&src);
        test_passthrough(&src, spdif_allowed[isink]);
        test_stereo_passthrough(&src);

        test_encode(&src, spdif_allowed[isink], can_encode);
        test_passthrough(&src, spdif_allowed[isink]);
        test_decode(&src);
      }
      else
      {
        test_decode(&src);
        test_encode(&src, spdif_allowed[isink], can_encode);
        test_stereo_passthrough(&src);
        test_decode(&src);
        test_stereo_passthrough(&src);
        test_encode(&src, spdif_allowed[isink], can_encode);
        test_decode(&src);
      }
      // have to drop sink because it will be deleted...
      dvd.set_sink(0);
    }

    Chunk chunk;
    while (!src.is_empty() || !f->is_empty())
    {
      if (f->is_empty())
      {
        if (!src.get_chunk(&chunk))
          return log->err_close("src.get_chunk() failed");
        if (!f->process(&chunk))
          return log->err_close("dvd.process() failed");
      }
      else
      {
        if (!f->get_chunk(&chunk))
          return log->err_close("dvd.get_chunk() failed");
      }
    }

    // todo: check number of output streams
    return log->close_group();
  }

  int test_decode(Source *src)
  {
    dvd.set_user(Speakers(FORMAT_PCM16, 0, 0));
    dvd.set_spdif(false, 0, false, false, false);
    return test_cycle("test_decode()", src, SPDIF_MODE_DISABLED, "decode", FORMAT_PCM16);
  }

  int test_passthrough(Source *src, bool spdif_allowed)
  {
    dvd.set_user(Speakers(FORMAT_PCM16, 0, 0));
    dvd.set_spdif(true, FORMAT_CLASS_SPDIFABLE, false, false, false);
    if (spdif_allowed)
      return test_cycle("test_passthrough()", src, SPDIF_MODE_PASSTHROUGH, "spdif passthrough", FORMAT_SPDIF);
    else
      return test_cycle("test_passthrough()", src, SPDIF_MODE_DISABLED, "sink refused", FORMAT_PCM16);
  }

  int test_encode(Source *src, bool spdif_allowed, bool can_encode)
  {
    dvd.set_user(Speakers(FORMAT_PCM16, 0, 0));
    dvd.set_spdif(true, 0, false, true, false);

    if (!can_encode)
      return test_cycle("test_encode()", src, SPDIF_MODE_DISABLED, "cannot encode", FORMAT_PCM16);

    if (spdif_allowed)
      return test_cycle("test_encode()", src, SPDIF_MODE_ENCODE, "ac3 encode", FORMAT_SPDIF);
    else
      return test_cycle("test_encode()", src, SPDIF_MODE_DISABLED, "sink refused", FORMAT_PCM16);
  }

  int test_stereo_passthrough(Source *src)
  {
    dvd.set_user(Speakers(FORMAT_PCM16, MODE_STEREO, 0));
    dvd.set_spdif(true, 0, false, true, true);
    return test_cycle("test_stereo_passthrough()", src, SPDIF_MODE_DISABLED, "stereo pcm passthrough", FORMAT_PCM16);
  }

  int test_cycle(const char *caller, Source *src, int status, const char *status_text, int out_format)
  {
    Chunk chunk;

    // process until status change
    // (filter may be either full or empty)

    while (!src->is_empty() && dvd.get_spdif_status() != status)
      if (f->is_empty())
      {
        if (!src->get_chunk(&chunk))
          return log->err("%s: src->get_chunk() failed before status change", caller);

        if (!f->process(&chunk))
          return log->err("%s: dvd.process() failed before status change", caller);

        chunk.set_dummy();
      }
      else
      {
        if (!f->get_chunk(&chunk))
          return log->err("%s: dvd.get_chunk() failed before status change", caller);
      }

    if (dvd.get_spdif_status() != status)
      return log->err("%s: cannot switch to %s state", caller, status_text);

    if (!chunk.is_dummy())
    {
      if (chunk.spk.format != out_format)
        return log->err("%s: incorrect output format", caller);
    }


    // ensure correct data generation in new state
    // (filter may be either full or empty)

    size_t data_size = 0;
    while (!src->is_empty() && data_size < min_data_size)
      if (f->is_empty())
      {
        if (!src->get_chunk(&chunk))
          return log->err("%s: src->get_chunk() failed after status change", caller);

        if (!f->process(&chunk))
          return log->err("%s: dvd.process() failed after status change", caller);
      }
      else
      {
        if (!f->get_chunk(&chunk))
          return log->err("%s: dvd.get_chunk() failed after status change", caller);

        if (!chunk.is_dummy())
        {
          if (chunk.spk.format != out_format)
            return log->err("%s: incorrect output format", caller);
          data_size += chunk.size;
        }
      }

    if (data_size < min_data_size)
      return log->err("Filter does not gernerate data!");

    // well done...
    // (filter may be either full or empty)
    return 0;
  }

  /////////////////////////////////////////////////////////////////////////////
  // Format agreement test
  //
  // Use following sink format restrictions:
  // * sink allows any format
  // * sink allows only pam16 and any number of channels
  // * sink allows only stereo and any format
  // * sink allows only stereo pcm16
  // * sink is fixed to quadro pcm24 and do not allow format changes
  //
  // Then enumerate all:
  // * user formats
  // * input formats
  //
  // Test is passed if output format is allowed by the sink for all
  // combinations of user and input formats for all sinks.

  void format_agreement()
  {
    int formats[] = 
    { 
      FORMAT_PCM16, FORMAT_PCM24, FORMAT_PCM32, FORMAT_PCMFLOAT, FORMAT_PCMDOUBLE, FORMAT_LINEAR, 
      FORMAT_PCM16_BE, FORMAT_PCM24_BE, FORMAT_PCM32_BE,
    };

    int masks[] = 
    { 
       0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 
      10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
      20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
      30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
      40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
      50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
      60, 61, 62, 63
    };

    int sample_rates[] = 
    {
      48000
    };

    Speakers sink_allow[] = 
    {
      Speakers(FORMAT_PCM24, MODE_QUADRO, 0),     // sink is fixed to 24bit quadro
      Speakers(FORMAT_UNKNOWN, MODE_STEREO, 0),   // sink accepts only stereo/any any format
      Speakers(FORMAT_UNKNOWN, 0, 0),             // sink accepts anything
      Speakers(FORMAT_PCM16, 0, 0),               // sink accepts only 16bit format/any channels
      Speakers(FORMAT_PCM16, MODE_STEREO, 0),     // sink accepts only 16bit stereo
    };

    Speakers sink_state[] = 
    {
      Speakers(FORMAT_PCM24, MODE_QUADRO, 48000), // sink is fixed to 24bit quadro
      Speakers(FORMAT_PCM16, MODE_STEREO, 48000), // sink accepts only stereo/any any format
      Speakers(FORMAT_PCM16, MODE_STEREO, 48000), // sink accepts anything
      Speakers(FORMAT_PCM16, MODE_STEREO, 48000), // sink accepts only 16bit format/any channels
      Speakers(FORMAT_PCM16, MODE_STEREO, 48000), // sink accepts only 16bit stereo
    };

    log->open_group("Format agreement test");

    for (int isink_format = 0; isink_format < array_size(sink_allow); isink_format++)
    {
      SinkAllow sink(sink_allow[isink_format]);
      sink.set_input(sink_state[isink_format]);
      log->msg("Trying sink with allowed format: %s %s %i", 
        sink_allow[isink_format].format_text(), 
        sink_allow[isink_format].mode_text(),
        sink_allow[isink_format].sample_rate);

      dvd.set_use_detector(false);
      dvd.set_spdif(false, 0, false, false, false);
      dvd.set_query_sink(true);
      dvd.set_sink(&sink);

      /////////////////////////////////////////////////////
      // User format enumeration
      // Note that user format must have zero sample rate
      for (int uformat = 0; uformat < array_size(formats); uformat++)
      for (int umask = 0; umask < array_size(masks); umask++)
      {
        // Statistics
        log->status("User format number %i of %i",
          uformat * array_size(masks) + umask, 
          array_size(formats) * array_size(masks)
        );

        // Set user format
        Speakers user_spk = Speakers(formats[uformat], masks[umask], 0);
        if (!dvd.set_user(user_spk))
        {
          log->err_close("Cannot set user format %s %s %i", 
            user_spk.format_text(), user_spk.mode_text(), user_spk.sample_rate);
          return;
        }

        /////////////////////////////////////////////////////
        // Input format enumeration
        // Note that input format must have non-zero mask
        for (int iformat = 0; iformat < array_size(formats); iformat++)
        for (int imask = 1; imask < array_size(masks); imask++)
        for (int isample_rate = 0; isample_rate < array_size(sample_rates); isample_rate++)
        {
          // Set input format
          Speakers in_spk = Speakers(formats[iformat], masks[imask], sample_rates[isample_rate]);
          if (!dvd.set_input(in_spk))
          {
            log->err_close("Cannot set input format %s %s %i", 
              in_spk.format_text(), in_spk.mode_text(), in_spk.sample_rate);
            // have to drop the sink because it will be destroyed
            dvd.set_sink(0);  
            return;
          }

          // Check sink compatibility
          if (!sink.query_input(dvd.get_output()))
          {
            log->err_close("Input format: %s %s %i; User format: %s %s %i; Output format set to %s %s %i (incompatible with the sink format)", 
              in_spk.format_text(), in_spk.mode_text(), in_spk.sample_rate,
              user_spk.format_text(), user_spk.mode_text(), user_spk.sample_rate,
              dvd.get_output().format_text(), dvd.get_output().mode_text(), dvd.get_output().sample_rate);
            // have to drop the sink because it will be destroyed
            dvd.set_sink(0);  
            return;
          }
        }
      }

      // have to drop the sink because it will be destroyed
      dvd.set_sink(0);  
    }

    log->close_group();
  }

};

///////////////////////////////////////////////////////////////////////////////
// Test function

int test_dvdgraph(Log *log)
{
  DVDGraph_test test(log);
  return test.test();
}
