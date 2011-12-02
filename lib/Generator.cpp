#include <math.h>
#include "Generator.h"

namespace AudioFilter {

///////////////////////////////////////////////////////////////////////////////
// Generator
// Base class for signal generation

Generator::Generator()
  :spk(Speakers::UNKNOWN), chunk_size(0), stream_len(0)
{
}

Generator::Generator(Speakers _spk, uint64_t _stream_len, size_t _chunk_size)
  :spk(Speakers::UNKNOWN), chunk_size(0), stream_len(0)
{
  init(_spk, _stream_len, _chunk_size);
}

bool Generator::init(Speakers _spk, uint64_t _stream_len, size_t _chunk_size)
{
  spk = Speakers::UNKNOWN;
  stream_len = 0;
  chunk_size = 0;

  if ( ! _chunk_size )
    return false;

  if ( ! querySpk(_spk) )
    return false;

  if ( _spk.isLinear() )
  {
    rawdata.free();
    if ( ! samples.allocate(_spk.getChannelCount(), _chunk_size) )
      return false;
  }
  else
  {
    samples.free();
    if ( ! rawdata.allocate(_chunk_size) )
      return false;
  }

  spk = _spk;
  chunk_size = _chunk_size;
  stream_len = _stream_len;
  return true;
}

// Source interface

Speakers Generator::getOutput(void) const
{
  return spk;
}

bool Generator::isEmpty(void) const
{
  return stream_len <= 0;
}

bool Generator::getChunk(Chunk *_chunk)
{
  bool eos = false;
  size_t n = chunk_size;

  if ( n >= stream_len )
  {
    n = (size_t)stream_len;
    stream_len = 0;
    eos = true;
  }
  else
  {
    stream_len -= n;
  }

  if ( spk.isLinear() )
  {
    genSamples(samples, n);
    _chunk->setLinear(spk, samples, n, false, 0, eos);
    return true;
  }
  else
  {
    genRawData(rawdata, n);
    _chunk->setRawData(spk, rawdata, n, false, 0, eos);
    return true;
  }
}

///////////////////////////////////////////////////////////////////////////////
// ZeroGen
// Silence generator

void ZeroGen::genSamples(samples_t samples, size_t n)
{
  for ( int ch = 0; ch < spk.getChannelCount(); ++ch )
    memset(samples[ch], 0, n * sizeof(sample_t));
}

void ZeroGen::genRawData(uint8_t *rawdata, size_t n)
{
  memset(rawdata, 0, n);
}

///////////////////////////////////////////////////////////////////////////////
// NoiseGen
// Noise generator

bool NoiseGen::init(Speakers _spk, int _seed, uint64_t _stream_len, size_t _chunk_size)
{
  rng.seed(_seed);
  return Generator::init(_spk, _stream_len, _chunk_size);
}

void NoiseGen::genSamples(samples_t samples, size_t n)
{
  for ( size_t i = 0; i < n; ++i )
  {
    for ( int ch = 0; ch < spk.getChannelCount(); ++ch )
      samples[ch][i] = rng.getSample();
  }
}

void NoiseGen::genRawData(uint8_t *rawdata, size_t n)
{
  rng.fillRaw(rawdata, n);
}

///////////////////////////////////////////////////////////////////////////////
// ToneGen
// Tone generator

bool ToneGen::querySpk(Speakers _spk) const
{
  return _spk.isLinear();
}

bool ToneGen::init(Speakers _spk, int _freq, double _phase, uint64_t _stream_len, size_t _chunk_size)
{
  phase = _phase;
  freq = _freq;
  return Generator::init(_spk, _stream_len, _chunk_size);
}

void ToneGen::genSamples(samples_t samples, size_t n)
{
  double w = 2 * M_PI * double(freq) / double(spk.getSampleRate());

  for ( size_t i = 0; i < n; ++i )
    samples[0][i] = sin(phase + i*w);

  phase += n*w;

  for ( int ch = 1; ch < spk.getChannelCount(); ++ch )
    memcpy(samples[ch], samples[0], n * sizeof(sample_t));
}

void ToneGen::genRawData(uint8_t *rawdata, size_t n)
{
  // never be here
  assert(false);
}

///////////////////////////////////////////////////////////////////////////////
// LineGen
// Line generator

bool LineGen::querySpk(Speakers _spk) const
{
  return _spk.isLinear();
}

bool LineGen::init(Speakers _spk, double _start, double _k, uint64_t _stream_len, size_t _chunk_size)
{
  phase = _start;
  k = _k;
  return Generator::init(_spk, _stream_len, _chunk_size);
}

void LineGen::genSamples(samples_t samples, size_t n)
{
  for ( size_t i = 0; i < n; ++i )
    samples[0][i] = phase + i*k;

  phase += n*k;

  for ( int ch = 1; ch < spk.getChannelCount(); ++ch )
    memcpy(samples[ch], samples[0], n * sizeof(sample_t));
}

void LineGen::genRawData(uint8_t *rawdata, size_t n)
{
  // never be here
  assert(false);
}

}; // namespace AudioFilter

// vim: ts=2 sts=2 et

