/*
  ParserFiler test

  * Timing test
    File for this test must contain at least 10 frames. It must be a file with
    constant bitrate (frame interval) and without stream changes.

*/

#include "log.h"
#include "source\generator.h"
#include "filters\parser_filter.h"
#include "parsers\ac3\ac3_parser.h"
#include "parsers\dts\dts_parser.h"
#include "parsers\mpa\mpa_parser.h"
#include "parsers\spdif\spdif_parser.h"

static const int seed = 4346;

class ParserFilter_test
{
protected:
  ParserFilter parser;
  AC3Parser ac3;
  DTSParser dts;
  MPAParser mpa;
  SPDIFParser spdif;

  Log *log;

public:
  ParserFilter_test(Log *_log): spdif(false)
  {
    log = _log;
  }

  int test()
  {
    log->open_group("ParserFilter timing test");
    test_timing("a.ac3.03f.ac3", &ac3);
    test_timing("a.dts.03f.dts", &dts);
    test_timing("a.mp2.005.mp2", &mpa);
    test_timing("a.ac3.03f.spdif", &spdif);
    test_timing("a.dts.03f.spdif", &spdif);
    test_timing("a.mp2.005.spdif", &spdif);
    return log->close_group();
  }

  void test_timing(const char *file_name, FrameParser *frame_parser)
  {
    log->msg("Timing test %s", file_name);

    parser.set_parser(frame_parser);

    // Load the file into memory

    MemFile f(file_name);
    if (!f)
    {
      log->err("Cannot open file %s", file_name);
      return;
    }

    // Detect frame interval

    uint8_t *ptr = f;
    uint8_t *end = ptr + f.size();

    StreamBuffer stream;
    stream.set_parser(frame_parser->header_parser());
    if (!stream.load_frame(&ptr, end))
    {
      log->err("Cannot determine frame interval for file %s", file_name);
      return;
    }

    if (f.size() < 10 * stream.get_frame_interval())
    {
      log->err("File %s is too small for the test", file_name);
      return;
    }

    // Work constants

    const size_t frame_interval = stream.get_frame_interval();
    const size_t max_frame_size = frame_parser->header_parser()->max_frame_size();

    // Prepare noise buffer
    NoiseGen noise(spk_rawdata, seed, int(max_frame_size * 2.5), int(max_frame_size * 2.5));
    Chunk noise_chunk;
    noise.get_chunk(&noise_chunk);

    uint8_t *noise_buf  = noise_chunk.rawdata;
    size_t noise_size = noise_chunk.size;

    // Chunk definitions

    struct InChunkDef
    {
      enum { chunk_noise, chunk_frame } type;
      float end_pos;

      bool    sync;
      vtime_t time;
    };

    InChunkDef input_chunks[] =
    {
      { InChunkDef::chunk_noise, 0.50, true,  1000 }, // chunk 1
      { InChunkDef::chunk_frame, 3.00, false, 0    }, // chunk 2
      { InChunkDef::chunk_noise, 1.00, false, 0    }, // chunk 3
      { InChunkDef::chunk_noise, 2.00, true,  2000 }, // chunk 4
      { InChunkDef::chunk_noise, 2.50, true,  3000 }, // chunk 5
      { InChunkDef::chunk_frame, 3.50, false, 0    }, // chunk 6
      { InChunkDef::chunk_frame, 5.50, true,  4000 }, // chunk 7
      { InChunkDef::chunk_frame, 5.75, true,  5000 }, // chunk 8
      { InChunkDef::chunk_frame, 8.25, true,  6000 }, // chunk 9
      { InChunkDef::chunk_frame, 8.75, true,  7000 }, // chunk 10
      { InChunkDef::chunk_frame, 9.00, true,  8000 }, // chunk 11
      { InChunkDef::chunk_frame, 10.00, true, 9000 }, // chunk 12
    };

    InChunkDef output_chunks[] =
    {
      { InChunkDef::chunk_frame, 1.0, true,  1000 },
      { InChunkDef::chunk_frame, 2.0, false, 0    },
      { InChunkDef::chunk_frame, 3.0, false, 0    },
      { InChunkDef::chunk_frame, 4.0, true,  3000 },
      { InChunkDef::chunk_frame, 5.0, true,  4000 },
      { InChunkDef::chunk_frame, 6.0, false, 0    },
      { InChunkDef::chunk_frame, 7.0, true,  6000 },
      { InChunkDef::chunk_frame, 8.0, false, 0    },
      { InChunkDef::chunk_frame, 9.0, false, 0    },
      { InChunkDef::chunk_frame, 10.0, true, 9000 },
    };

    // Test

    int noise_pos = 0;
    int frame_pos = 0;
    Chunk chunk;
    int i = 0; // input chunk index
    int o = 0; // output chunk index
 
    for (i = 0; i < array_size(input_chunks); i++)
    {

      // Fill the chunk

      switch (input_chunks[i].type)
      {
        case InChunkDef::chunk_frame:
        {
          int new_frame_pos = (int)(input_chunks[i].end_pos * frame_interval);
          assert(new_frame_pos - frame_pos > 0);
          chunk.set_rawdata(
            spk_rawdata, f + frame_pos, new_frame_pos - frame_pos, 
            input_chunks[i].sync, input_chunks[i].time);
          frame_pos = new_frame_pos;
          break;
        }

        case InChunkDef::chunk_noise:
        {
          int new_noise_pos = (int)(input_chunks[i].end_pos * max_frame_size);
          assert(new_noise_pos - noise_pos > 0);
          chunk.set_rawdata(
            spk_rawdata, noise_buf + noise_pos, new_noise_pos - noise_pos, 
            input_chunks[i].sync, input_chunks[i].time);
          noise_pos = new_noise_pos;
          break;
        }

        default:
          assert(false);
      }

      // Process the chunk

      if (!parser.process(&chunk))
      {
        log->err("Processing error");
        return;
      }

      // Verify output

      while (!parser.is_empty())
      {
        parser.get_chunk(&chunk);

        if (chunk.is_dummy())
          continue;

        if (chunk.size == 0)
          continue;

        if (chunk.sync != output_chunks[o].sync || 
            chunk.time != output_chunks[o].time)
        {
          log->err("Timing error. Must be (sync: %s, time: %i), but got (sync: %s, time: %i)",
            (output_chunks[o].sync? "true": "false"), (int)output_chunks[o].time,
            (chunk.sync? "true": "false"), (int)chunk.time);
          return;
        }
        o++;
      } // while (!parser.is_empty())
    } // for (int i = 0; i < array_size(input_chunks); i++)

    if (o < array_size(output_chunks))
    {
      log->err("Not all output chunks generated. Must be %i, but %i generated",
        array_size(output_chunks), o);
      return;
    }
    
    return;
  }
};

///////////////////////////////////////////////////////////////////////////////
// Test function
///////////////////////////////////////////////////////////////////////////////

int test_parser_filter(Log *log)
{
  ParserFilter_test test(log);
  return test.test();
}
