#include <fcntl.h>
//#include <io.h>
#include <cstdio>
#include <iostream>

#include <AudioFilter/FileParser.h>
#include <AudioFilter/SpdifWrapper.h>
#include <AudioFilter/RawSink.h>
#include <AudioFilter/WavSink.h>
#include <AudioFilter/VTime.h>

using namespace AudioFilter;

int main(int argc, const char **argv)
{
  if ( argc < 3 )
  {
    std::cout <<
"Spdifer\n"
"=======\n"
"This utility encapsulates AC3/DTS/MPEG Audio stream into SPDIF stream\n"
"according to IEC 61937\n"
"\n"
"This utility is a part of AudioFilter project (git://github.com/cbxbiker61/audiofilter.git)\n"
"Copyright (c) 2007-2009 by Alexander Vigovsky\n"
"Copyright (c) 2011 Kelly Anderson\n"
"\n"
"Usage:\n"
"  spdifer input_file output_file [-raw | -wav] [hdfreqMult]\n"
"\n"
"Options:\n"
"  input_file  - file to convert\n"
"  output_file - file to write result to\n"
"  -raw - make raw SPDIF stream output (default)\n"
"  -wav - make PCM WAV file with SPDIF data (for writing to CD Audio)" << std::endl;
    return 0;
  }

  /////////////////////////////////////////////////////////
  // Parse command line

  const char *infile(argv[1]);
  const char *outfile = argv[2];
  Sink *sink = ( argc <= 3 || strcmp(argv[3], "-wav") )
    ? (Sink*)(new RawSink(outfile))
    : (Sink*)(new WavSink(outfile));
  int hdFreqMult(4);
  if ( argc >= 5 )
    hdFreqMult = 8;

  MultiHeaderParser *mhp(new MultiHeaderParser());
  mhp->addParser(new SpdifHeaderParser());
  mhp->addParser(new DtsHeaderParser());
  mhp->addParser(new MpaHeaderParser());
  mhp->addParser(new Ac3HeaderParser());
  FileParser *fp(new FileParser(infile, mhp));
  SpdifWrapper spdif(mhp, hdFreqMult);

  int streams(0);
  int frames(0);
  int skipped(0);
  int errors(0);
  size_t size(0);
  vtime_t time(getLocalTime());
  vtime_t old_time(time);
  vtime_t start_time(time);

  Chunk chunk;

  while ( ! fp->eof() )
  {
    if ( fp->loadFrame() )
    {
      ++frames;

      if ( fp->isNewStream() )
        ++streams;

      if ( spdif.parseFrame(fp->getFrame(), fp->getFrameSize()) )
      {
        Speakers spk = spdif.getSpk();

        if ( spk.format == FORMAT_SPDIF )
        {
          chunk.setRawData(
                Speakers(FORMAT_PCM16, MODE_STEREO, spk.sample_rate)
                , spdif.getRawData(), spdif.getRawSize());

          if ( sink->process(&chunk) )
            size += chunk.size;
          else
            ++errors;
        }
        else
          ++skipped;
      }
      else
      {
        ++errors;
      }
    }

    time = getLocalTime();

    if ( fp->eof() || time > old_time + 0.1 )
    {
      char info[256];
      old_time = time;

      float fileSize = float(fp->getSize());
      float processed = float(fp->getPos());
      float elapsed   = float(old_time - start_time);
      int eta = int((fileSize / processed - 1.0) * elapsed);

      snprintf(info, sizeof(info), "%02i%% %02i:%02i In: %iM (%2iMB/s) Out: %liM Frames: %i (%s)       \r"
        , int(processed * 100 / fileSize)
        , eta / 60
        , eta % 60
        , int(processed / 1000000)
        , int(processed/elapsed/1000000)
        , (long)(size / 1000000)
        , frames
        , fp->isInSync() ? fp->getSpk().getFormatText(): "Unknown");

      std::cerr << info << std::flush;
    }
  }

  // flush output
  chunk.setEmpty(sink->getInput(), false, 0, true);
  sink->process(&chunk);

  delete fp;
  delete mhp;
  delete sink;

  // Final notes
  std::cout << std::endl;

  if ( streams > 1 )
    std::cout << streams << " streams converted" << std::endl;

  if ( skipped )
    std::cout << skipped << " frames skipped" << std::endl;

  if ( errors )
    std::cout << errors << " frame conversion errors" << std::endl;

  return 0;
}

// vim: ts=2 sts=2 et

