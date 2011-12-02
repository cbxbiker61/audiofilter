#include <math.h>
#include <iostream>
#include <AudioFilter/WavSink.h>
#include <AudioFilter/VTime.h>
#include <AudioFilter/VArgs.h>
#include <AudioFilter/GainFilter.h>
#include <WavSource.h>
#include "FilterGraph.h"
#include "AgcFilter.h"
#include "filters/Converter.h"

using namespace AudioFilter;

const int block_size(65536);

int main(int argc, char **argv)
{
  if ( argc < 3 )
  {
    std::cout <<
"WAVDiff\n"
"=======\n"
"Find the difference between 2 audio files. Calculates amplitude difference\n"
"and RMS difference. Can compare files of different types, like PCM16 and\n"
"PCM Float (number of channels and sample rate must match). Can write\n"
"the difference into the difference file. May check maximum difference for\n"
"automated testing\n"
"\n"
"This utility is a part of AC3Filter project (http://ac3filter.net)\n"
"Copyright (c) 2008-2009 by Alexander Vigovsky\n"
"\n"
"Usage:\n"
"  > wavdiff a.wav b.wav [-diff c.wav] [-max_diff:n] [-max_rms:n] [-max_mean:n]\n"
"\n"
"Options:\n"
"  a.wav - First file to compare\n"
"  b.wav - Second file to compare\n"
"  c.wav - Difference file\n"
"  -max_diff - maximum difference between files allowed (dB)\n"
"  -max_rms  - maximum RMS difference between files allowed (dB)\n"
"  -max_mean - maximum mean difference between files allowed (dB)\n"
"\n"
"Example:\n"
" > wavdiff a.wav b.wav -diff c.wav -max_rms:-90\n"
" Comapre a.wav and b.wav and write the difference into c.wav\n"
" If the difference between files is more than -90dB exit code will be non-zero" << std::endl;
    return 0;
  }

  const char *filename1(argv[1]);
  const char *filename2(argv[2]);
  const char *diff_filename(0);
  bool check_diff(false);
  bool check_rms(false);
  bool check_mean(false);
  double max_diff(0);
  double max_rms(0);
  double max_mean(0);

  /////////////////////////////////////////////////////////////////////////////
  // Parse arguments

  for ( int iarg = 3; iarg < argc; ++iarg )
  {
    if ( isArg(argv[iarg], "max_diff", argt_num) )
    {
      max_diff = getNumArg(argv[iarg]);
      check_diff = true;
      continue;
    }

    if ( isArg(argv[iarg], "max_mean", argt_num) )
    {
      max_mean = getNumArg(argv[iarg]);
      check_mean = true;
      continue;
    }

    if ( isArg(argv[iarg], "max_rms", argt_num) )
    {
      max_rms = getNumArg(argv[iarg]);
      check_rms = true;
      continue;
    }

    if ( isArg(argv[iarg], "diff", argt_exist) )
    {
      if ( argc - iarg < 1 )
      {
        std::cout << "-diff : specify a file name" << std::endl;
        return 1;
      }

      diff_filename = argv[++iarg];
      continue;
    }

    std::cout << "Error: unknown option: " << argv[iarg] << std::endl;
    return -1;
  }

  std::cout << "Comparing " << filename1 << " - " << filename2 << "..." << std::endl;

  /////////////////////////////////////////////////////////////////////////////
  // Open files

  WavSource f1(filename1, block_size);

  if ( ! f1.isOpen() )
  {
    std::cout << "Error: cannot open file: " << filename1 << std::endl;
    return -1;
  }

  WavSource f2(filename2, block_size);

  if ( ! f2.isOpen() )
  {
    std::cout << "Error: cannot open file: " << filename2 << std::endl;
    return -1;
  }

  WavSink f_diff;

  if ( diff_filename )
  {
    if ( ! f_diff.open(diff_filename) )
    {
      std::cout << "Error: cannot open file: " << diff_filename << std::endl;
      return -1;
    }
  }

  Speakers spk1 = f1.getOutput();
  Speakers spk2 = f2.getOutput();
  Speakers spk_diff = spk1;

  if ( spk1.getFormat() == FORMAT_PCM16 )
    spk_diff = spk2;
  else if ( spk2.getFormat() == FORMAT_PCM16 )
    spk_diff = spk1;
  else if ( spk1.getFormat() == FORMAT_PCM24 )
    spk_diff = spk2;
  else if ( spk2.getFormat() == FORMAT_PCM24 )
    spk_diff = spk1;
  else if ( spk1.getFormat() == FORMAT_PCM32 )
    spk_diff = spk2;

  /////////////////////////////////////////////////////////////////////////////
  // Build the chain

  Converter conv1(block_size);
  Converter conv2(block_size);
  Converter conv_diff(block_size);
  conv1.setFormat(FORMAT_LINEAR);
  conv2.setFormat(FORMAT_LINEAR);
  conv_diff.setFormat(spk_diff.getFormat());

  if ( spk1.getChannelCount() != spk2.getChannelCount() )
  {
    std::cout << "Error: different number of channels" << std::endl;
    return -1;
  }

  if ( spk1.getSampleRate() != spk2.getSampleRate() )
  {
    std::cout << "Error: different sample rates" << std::endl;
    return -1;
  }

  if ( ! conv1.setInput(spk1) )
  {
    std::cout << "Error: unsupported file type: " << filename1 << std::endl;
    return -1;
  }

  if ( ! conv2.setInput(spk2) )
  {
    std::cout << "Error: unsupported file type: " << filename2 << std::endl;
    return -1;
  }

  SourceFilter src1(&f1, &conv1);
  SourceFilter src2(&f2, &conv2);
  SinkFilter sink_diff(&f_diff, &conv_diff);

  /////////////////////////////////////////////////////////////////////////////
  // Do the job

  Chunk chunk1;
  Chunk chunk2;
  Chunk chunk_diff;
  size_t n = 0;

  double diff(0.0);
  double mean(0.0);
  double rms(0.0);

  Speakers spk;
  //size_t len;

  vtime_t t = getLocalTime() + 0.1;

  for ( ; ; )
  {
    if ( ! chunk1.size )
    {
      if ( src1.isEmpty() )
        break;

      if ( ! src1.getChunk(&chunk1) )
      {
        std::cout << "src1.getChunk() fails" << std::endl;
        return -1;
      }

      if ( chunk1.isDummy() )
        continue;
    }

    if ( ! chunk2.size )
    {
      if ( src2.isEmpty() )
        break;

      if ( ! src2.getChunk(&chunk2) )
      {
        std::cout << "src2.getChunk() fails" << std::endl;
        return -1;
      }

      if ( chunk2.isDummy() )
        continue;
    }

    ///////////////////////////////////////////////////////
    // Compare data

    size_t len(MIN(chunk2.size, chunk1.size));
    double norm1(1.0 / spk1.getLevel());
    double norm2(1.0 / spk2.getLevel());

    for ( int ch = 0; ch < spk1.getChannelCount(); ++ch )
    {
      for ( size_t s = 0; s < len; ++s )
      {
        sample_t d = chunk1.samples[ch][s] * norm1 - chunk2.samples[ch][s] * norm2;

        if ( fabs(d) > diff )
          diff = fabs(d);

        mean += fabs(d);
        rms += d * d;
      }
    }

    ///////////////////////////////////////////////////////
    // Write difference file

    if ( f_diff.isOpen() )
    {
      Speakers spk_diff_linear = spk_diff;
      spk_diff_linear.setLinear();

      double norm1(spk_diff.getLevel() / spk1.getLevel());
      double norm2(spk_diff.getLevel() / spk2.getLevel());

      for ( int ch = 0; ch < spk1.getChannelCount(); ++ch )
      {
        for ( size_t s = 0; s < len; ++s )
          chunk1.samples[ch][s] = chunk1.samples[ch][s] * norm1 - chunk2.samples[ch][s] * norm2;
      }

      chunk_diff.setLinear(spk_diff_linear, chunk1.samples, len);

      if ( ! sink_diff.process(&chunk_diff) )
      {
        std::cout << "Error: cannot write to the difference file" << std::endl;
        return -1;
      }
    }

    n += len;
    chunk1.drop(len);
    chunk2.drop(len);

    ///////////////////////////////////////////////////////
    // Statistics

    if ( getLocalTime() > t )
    {
      t += 0.1;
      double pos = double(f1.pos()) * 100 / f1.size();
      std::cout << (int)pos << "%\r" << std::flush;
    }

  } // while (1)

  if ( f_diff.isOpen() )
  {
    chunk_diff.setEmpty(spk_diff);
    chunk_diff.setEos();

    if ( ! sink_diff.process(&chunk_diff) )
    {
      std::cout << "Error: cannot write to the difference file" << std::endl;
      return -1;
    }
  }

  rms = sqrt(rms/n);
  mean /= n;

  std::cout << "    \r" << std::flush;
  std::cout << "Max diff: " << value2db(diff) << "dB" << std::endl;
  std::cout << "RMS diff: " << value2db(rms) << "dB" << std::endl;
  std::cout << "Mean diff: " << value2db(mean) << "dB" << std::endl;

  int result = 0;

  if ( check_diff && diff > db2value(max_diff) )
  {
  std::cout << "Error: maximum difference is too large" << std::endl;
    result = -1;
  }

  if ( check_rms && rms > db2value(max_rms) )
  {
    std::cout << "Error: RMS difference is too large" << std::endl;
    result = -1;
  }

  if ( check_mean && mean > db2value(max_mean) )
  {
    std::cout << "Error: mean difference is too large" << std::endl;
    result = -1;
  }

  return result;
}

// vim: ts=2 sts=2 et

