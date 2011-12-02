#include <iostream>
#include <exception>
#ifdef __unix__
#include <cstdlib>
#endif
#include <Generator.h>
#ifdef __WIN__
#include <sink/sink_dsound.h>
#endif
#include <AudioFilter/RawSink.h>
#include <AudioFilter/WavSink.h>
#include <AudioFilter/VArgs.h>

using namespace AudioFilter;

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

class RuntimeException : public std::exception
{
public:
	RuntimeException(std::string);
	~RuntimeException() throw() {}
	virtual const char* what() const throw();
	std::string toString(void);
private:
	std::string msg;
};

RuntimeException::RuntimeException( std::string _msg )
	: exception(), msg(_msg)
{
}

const char* RuntimeException::what() const throw()
{
	return msg.c_str();
}

std::string RuntimeException::toString(void)
{
	return msg;
}

int run(int argc, char **argv);

int main(int argc, char **argv)
{
  try
  {
    return run(argc, argv);
  }
  catch (const std::exception &e)
  {
    std::cerr << e.what() << std::endl;
  }
  return 1;
}

int run(int argc, char **argv)
{
  if ( argc < 2 )
  {
    std::cout <<
"Noise generator\n"
"This utility is part of AC3Filter project (http://ac3filter.net)\n"
"Copyright (c) 2007-2009 by Alexander Vigovsky\n\n"

"Usage:\n"
"  noise ms [options]\n"
"\n"
"Options:\n"
"  ms - length in milliseconds\n"
"\n"
"  output mode:\n"
"    -p[lay] - playback (*)\n"
"    -w[av] file.wav - write output to a WAV file\n"
"    -r[aw] file.raw - write output to a RAW file\n"
"\n"
"  format:\n"
"    -spk:n - set number of output channels:\n"
"        1 - 1/0 (mono)     4 - 2/2 (quadro)\n"
"    (*) 2 - 2/0 (stereo)   5 - 3/2 (5 ch)\n"
"        3 - 3/0 (surround) 6 - 3/2+SW (5.1)\n"
"    -fmt:n - set sample format:\n"
"    (*) 0 - PCM 16         3 - PCM 16 (big endian)\n"
"        1 - PCM 24         4 - PCM 24 (big endian)\n"
"        2 - PCM 32         5 - PCM 32 (big endian)\n"
"                           6 - PCM Float\n"
"  -rate:n - sample rate (default is 48000)\n"
"\n"
"  -seed:n - seed for the random numbers generator\n"
"\n"
"Example:\n"
"  > noise 1000 -w noise.wav -fmt:2 -seed:666\n"
"  Make 1sec stereo PCM Float noise file" << std::endl;
    return -1;
  }

#ifdef __WIN__
  DSoundSink dsound;
#else
#ifdef __linux__disabled
  PulseSink pulse;
#endif
#endif
  Sink *sink(0);
  int imask(2);
  int iformat(0);
  int sample_rate(48000);
  int seed(0);

  /////////////////////////////////////////////////////////
  // Parse arguments
  /////////////////////////////////////////////////////////

  int ms = atoi(argv[1]);

  for ( int iarg = 2; iarg < argc; ++iarg )
  {
    ///////////////////////////////////////////////////////
    // Output format
    ///////////////////////////////////////////////////////

    // -spk - number of speakers
    if ( isArg(argv[iarg], "spk", argt_num) )
    {
      imask = int(getNumArg(argv[iarg]));

      if ( imask < 1 || imask > ((int)array_size(mask_tbl)) )
      {
        throw RuntimeException("-spk : incorrect speaker configuration");
      }

      continue;
    }

    // -fmt - sample format
    if ( isArg(argv[iarg], "fmt", argt_num) )
    {
      iformat = int(getNumArg(argv[iarg]));

      if ( iformat < 0 || iformat > ((int)array_size(format_tbl)) )
      {
        throw RuntimeException("-fmt : incorrect sample format");
      }

      continue;
    }

    // -rate - sample rate
    if ( isArg(argv[iarg], "rate", argt_num) )
    {
      sample_rate = int(getNumArg(argv[iarg]));

      if ( sample_rate < 0 )
      {
        throw RuntimeException("-rate : incorrect sample rate");
      }

      continue;
    }

    ///////////////////////////////////////////////////////
    // Output mode
    ///////////////////////////////////////////////////////

    // -p[lay] - play
    if ( isArg(argv[iarg], "p", argt_exist )
        || isArg( argv[iarg], "play", argt_exist) )
    {
      if ( sink )
      {
        throw RuntimeException("-play : ambigous output mode");
      }

#ifdef __WIN__
      sink = new DSoundSink();
#else
#ifdef __linux__disabled
      sink = new PulseSink();
#endif
#endif
      continue;
    }

    // -r[aw] - RAW output
    if ( isArg(argv[iarg], "r", argt_exist) ||
        isArg(argv[iarg], "raw", argt_exist) )
    {
      if ( sink )
      {
        throw RuntimeException("-raw : ambigous output mode");
      }

      if ( (argc - iarg) < 1 )
      {
        throw RuntimeException("-raw : specify a filename");
      }

      const char *filename = argv[++iarg];
      sink = new RawSink(filename);
      continue;
    }

    // -w[av] - WAV output
    if ( isArg(argv[iarg], "w", argt_exist)
        || isArg(argv[iarg], "wav", argt_exist) )
    {
      if ( sink )
      {
        throw RuntimeException("-wav : ambigous output mode");
      }

      if ( argc - iarg < 1 )
      {
        throw RuntimeException("-wav : specify a filename");
      }

      const char *filename = argv[++iarg];
      sink = new WavSink(filename);
      continue;
    }

    // -seed - RNG seed
    if ( isArg(argv[iarg], "seed", argt_num) )
    {
      seed = int(getNumArg(argv[iarg]));

      if ( sample_rate < 0 )
      {
        throw RuntimeException("-rate : incorrect sample rate");
      }

      continue;
    }

    throw RuntimeException(std::string("Error: unknown option: ") + argv[iarg]);
  }

  /////////////////////////////////////////////////////////
  // Setup output
  /////////////////////////////////////////////////////////

#ifdef __WIN__
  if ( ! sink || sink == &dsound )
  {
    sink = &dsound;
    dsound.openDsound(0);
  }
#else
#if __linux__disabled
  if ( ! sink || sink == &pulse )
  {
    sink = &pulse;
    pulse.openPulse();
  }
#endif
#endif

  Speakers spk(format_tbl[iformat], mask_tbl[imask], sample_rate);

  std::cout << "Opening " << spk.getFormatText()
          << " " << spk.getModeText()
          << " " << spk.getSampleRate() << " Hz audio output..."
          << std::endl;

  if ( ! sink->setInput(spk) )
  {
    throw RuntimeException("Error: Cannot open audio output!");
  }

  /////////////////////////////////////////////////////////
  // Process
  /////////////////////////////////////////////////////////

  double size = double(spk.getSampleSize() * spk.getChannelCount() * spk.getSampleRate()) * double(ms) / 1000;
  NoiseGen noise(spk, seed, (uint64_t) size);
  Chunk chunk;

  do
  {
    if ( noise.getChunk(&chunk) )
    {
      if ( sink->process(&chunk) )
      {
        continue;
      }
      else
      {
        throw RuntimeException("sink.process() failed");
      }
    }
    else
    {
      throw RuntimeException("noise.get_chunk() failed");
    }

  } while ( ! chunk.eos );

  if ( sink )
    delete sink;

  return 0;
}

// vim: ts=2 sts=2 et

