#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <AudioFilter/MpegDemux.h>
#include <AudioFilter/VArgs.h>
#include <AudioFilter/VTime.h>

using namespace AudioFilter;

const int buf_size(65536);

///////////////////////////////////////////////////////////////////////////////
// File information
///////////////////////////////////////////////////////////////////////////////

const char *stream_type(int stream)
{
  if ( stream == 0xbc )
    return "Reserved stream";
  else if ( stream == 0xbe )
    return "Padding stream";
  else if ( stream == 0xbd )
    return "Private stream 1";
  else if ( stream == 0xbf )
    return "Private stream 2";
  else if ( (stream & 0xe0) == 0xc0 )
    return "MPEG Audio stream";
  else if ( (stream & 0xf0) == 0xe0 )
    return "MPEG Video stream";
  else if ( (stream & 0xf0) == 0xf0 )
    return "Reserved data stream";

  return "unknown stream type";
}
const char *substream_type(int substream)
{
  if ( (substream & 0xf8) == 0x80 )
    return "AC3 Audio substream";
  else if ( (substream & 0xf8) == 0x88 )
    return "DTS Audio substream";
  else if ( (substream & 0xf0) == 0xa0 )
    return "LPCM Audio substream";
  else if ( (substream & 0xf0) == 0x20 )
    return "Subtitle substream";
  else if ( (substream & 0xf0) == 0x30 )
    return "Subtitle substream";

  return "unknown substream type";
}

void info(FILE *f)
{
  uint8_t buf[buf_size];
  uint8_t *data;
  uint8_t *end;

  int stream[256];
  int substream[256];

  memset(stream, 0, sizeof(stream));
  memset(substream, 0, sizeof(substream));

  PSParser ps;
  ps.reset();

  while ( ! feof(f) && (ftell(f) < 1024 * 1024) ) // analyze only first 1MB
  {
    // read data from file
    data = buf;
    end = buf;
    end += fread(buf, 1, buf_size, f);

    while ( data < end )
    {
      if ( ps.getPayloadSize() )
      {
        if ( ! stream[ps.getStream()] )
        {
          ++stream[ps.getStream()];
          printf("Found stream %x (%s)      \n"
                  , ps.getStream(), stream_type(ps.getStream()));
        }

        if ( ps.getStream() == 0xBD && ! substream[ps.getSubStream()] )
        {
          ++substream[ps.getSubStream()];
          printf("Found substream %x (%s)   \n"
                  , ps.getSubStream(), substream_type(ps.getSubStream()));
        }

        size_t len = MIN(ps.getPayloadSize(), size_t(end - data));
        ps.subtractFromPayloadSize(len);
        data += len;
      }
      else
      {
        ps.parse(&data, end);
      }
    }
  }

  if ( ps.getErrors() )
    printf("Stream contains errors (not a MPEG program stream?)\n");
}

///////////////////////////////////////////////////////////////////////////////
// Demux
///////////////////////////////////////////////////////////////////////////////

void demux(FILE *f, FILE *out, int stream, int substream, bool pes)
{
  uint8_t buf[buf_size];
  uint8_t *data;
  uint8_t *end;

  vtime_t start_time;
  vtime_t old_time;

  PSParser ps;
  ps.reset();

  /////////////////////////////////////////////////////////
  // Determine file size

  long start_pos = ftell(f);
  fseek(f, 0, SEEK_END);
  long file_size = ftell(f);
  fseek(f, start_pos, SEEK_SET);

  old_time = 0;
  start_time = getLocalTime();

  while ( ! feof(f) )
  {
    ///////////////////////////////////////////////////////
    // Read data from file

    data = buf;
    end = buf;
    end += fread(buf, 1, buf_size, f);

    ///////////////////////////////////////////////////////
    // Demux data

	bool isHeaderWritten = false;

    while ( data < end )
    {
      if ( ps.getPayloadSize() )
      {
        size_t len = MIN(ps.getPayloadSize(), size_t(end - data));

        // drop other streams
        if ( (stream && stream != ps.getStream())
            || (stream && substream && substream != ps.getSubStream()) )
        {
          ps.subtractFromPayloadSize(len);
          data += len;
          continue;
        }

        // write packet header
        if ( pes && ! isHeaderWritten && ps.getHeaderSize() )
        {
          fwrite(ps.getHeader(), 1, ps.getHeaderSize(), out);
          isHeaderWritten = true; // do not write header anymore
        }

        // write packet payload
        fwrite(data, 1, len, out);
        ps.subtractFromPayloadSize(len);
        data += len;
      }
      else
        ps.parse(&data, end);
    }

    ///////////////////////////////////////////////////////
    // Statistics (10 times/sec)

    if ( feof(f) || getLocalTime() > old_time + 0.1 )
    {
      old_time = getLocalTime();
      long in_pos = ftell(f);
      long out_pos = ftell(out);

      float processed = float(in_pos - start_pos);
      float elapsed = float(old_time - start_time);

      int eta = int((float(file_size) / processed - 1.0) * elapsed);

      printf("%02i:%02i %02i%% In: %liM (%2iMB/s) Out: %liK    ",
        eta / 60, eta % 60,
        int(processed * 100 / float(file_size)),
        in_pos / 1000000,
        int(processed/elapsed/1000000),
        out_pos / 1000);

      if ( stream ? substream : ps.getSubStream() )
        printf(" stream:%02x/%02x\r", stream? stream: ps.getStream(), stream ? substream : ps.getSubStream());
      else
        printf(" stream:%02x   \r", stream ? stream: ps.getStream());
    }
  }

  printf("\n");

  if ( ps.getErrors() )
    printf("Stream contains errors (not a MPEG program stream?)\n");

};

///////////////////////////////////////////////////////////////////////////////
// Main
///////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
  int stream = 0;
  int substream = 0;

  char *filename = 0;
  char *filename_out = 0;

  FILE *f = 0;
  FILE *out = 0;

  printf("MPEG Program Stream demuxer\n"
         "This utility is a part of AC3Filter project (http://ac3filter.net)\n"
         "Copyright (c) 2007-2009 by Alexander Vigovsky\n\n");
  if (argc < 2)
  {
    printf("Usage:\n"
           "  mpeg_demux file.mpg [-i] [-d | -p output_file [-s=x | -ss=x]]\n"
           "  -i - file info (default)\n"
           "  -d - demux to elementary stream\n"
           "  -p - demux to pes stream\n"
           "  -s=xx  - demux stream xx (hex)\n"
           "  -ss=xx - demux substream xx (hex)\n"
           "\n"
           "Note: if stream/substream is not specified, demuxer will dump contents of all\n"
           "packets found. This mode is useful to demux PES streams (not multiplexed) even\n"
           "with format changes.\n"
           );
    exit(0);
  }

  int  iarg = 0;
  enum { mode_none, mode_info, mode_es, mode_pes } mode = mode_none;

  // Parse arguments

  filename = argv[1];

  for ( iarg = 2; iarg < argc; ++iarg )
  {
    // -i - info
    if ( isArg(argv[iarg], "i", argt_exist) )
    {
      if ( mode != mode_none )
      {
        printf("-i : ambigous mode\n");
        return 1;
      }

      mode = mode_info;
      continue;
    }

    // -d - demux to es
    if ( isArg(argv[iarg], "d", argt_exist) )
    {
      if ( argc - iarg < 2 )
      {
        printf("-d : specify a file name\n");
        return 1;
      }

      if ( mode != mode_none )
      {
        printf("-d : ambigous mode\n");
        return 1;
      }

      mode = mode_es;
      filename_out = argv[++iarg];
      continue;
    }

    // -p - demux to pes
    if ( isArg(argv[iarg], "p", argt_exist) )
    {
      if ( argc - iarg < 2 )
      {
        printf("-p : specify a file name\n");
        return 1;
      }

      if ( mode != mode_none )
      {
        printf("-p : ambigous mode\n");
        return 1;
      }

      mode = mode_pes;
      filename_out = argv[++iarg];
      continue;
    }

    // -stream - stream to demux
    if ( isArg(argv[iarg], "s", argt_hex) )
    {
      stream = getHexArg(argv[iarg]);
      continue;
    }

    // -substream - substream to demux
    if ( isArg(argv[iarg], "ss", argt_hex) )
    {
      substream = getHexArg(argv[iarg]);
      continue;
    }

    printf("Unknown parameter: %s\n", argv[iarg]);
    return 1;
  }

  if ( substream )
  {
    if ( stream && stream != 0xbd )
    {
      printf("Cannot demux substreams for stream 0x%x\n", stream);
      return 1;
    }
    stream = 0xbd;
  }

  if ( ! (f = fopen(filename, "rb")) )
  {
    printf("Cannot open file %s for reading\n", filename);
    return 1;
  }

  if ( filename_out )
    if (!(out = fopen(filename_out, "wb")))
    {
      printf("Cannot open file %s for writing\n", filename_out);
      return 1;
    }

  // Do the job

  switch ( mode )
  {
  case mode_none:
  case mode_info:
    info(f);
    break;

  case mode_es:
    demux(f, out, stream, substream, false);
    break;

  case mode_pes:
    demux(f, out, stream, substream, true);
    break;
  }

  // Finish

  fclose(f);

  if ( out )
    fclose(out);

  return 0;
}

