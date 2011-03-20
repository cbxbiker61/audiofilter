#pragma once
#ifndef AUDIOFILTER_GENERATOR_H
#define AUDIOFILTER_GENERATOR_H

/*
 * Signal generation sources
 * Generator class is an abstract base for generators:
 *   Silence generator (ZeroGen)
 *   Noise generator (NoiseGen)
 *   Sine wave generator (SineGen)
 *   Line generator (LinGen)
 */

#include <AudioFilter/Filter.h>
#include <AudioFilter/Buffer.h>
#include <AudioFilter/Rng.h>

namespace AudioFilter {

class Generator : public Source
{
public:
  size_t getChunkSize(void) const
  {
    return chunk_size;
  }

  uint64_t getStreamLen(void) const
  {
    return stream_len;
  }

  // Source interface
  virtual Speakers getOutput(void) const;
  virtual bool isEmpty(void) const;
  virtual bool getChunk(Chunk *chunk);

protected:

  Generator();
  Generator(Speakers spk, uint64_t stream_len, size_t chunk_size = 4096);

  virtual bool querySpk(Speakers spk) const
  {
    return true;
  }

  virtual void genSamples(samples_t samples, size_t n)
  {
    assert(false);
  }

  virtual void genRawData(uint8_t *rawdata, size_t n)
  {
    assert(false);
  }

  bool init(Speakers spk, uint64_t stream_len, size_t chunk_size = 4096);

  Speakers  spk;
  SampleBuf samples;
  UInt8Buf  rawdata;
  size_t    chunk_size;
  uint64_t  stream_len;
};

class ZeroGen : public Generator
{
public:
  ZeroGen() {}
  ZeroGen(Speakers spk, size_t stream_len, size_t chunk_size = 4096)
    :Generator(spk, stream_len, chunk_size)
  {}

  bool init(Speakers spk, uint64_t stream_len, size_t chunk_size = 4096)
  {
    return Generator::init(spk, stream_len, chunk_size);
  }

protected:
  virtual void genSamples(samples_t samples, size_t n);
  virtual void genRawData(uint8_t *rawdata, size_t n);
};

class NoiseGen : public Generator
{
public:
  NoiseGen() {}
  NoiseGen(Speakers spk, int seed, uint64_t stream_len, size_t chunk_size = 4096)
    :Generator(spk, stream_len, chunk_size), rng(seed)
  {}

  bool init(Speakers spk, int seed, uint64_t stream_len, size_t chunk_size = 4096);

protected:
  RNG rng;
  virtual void genSamples(samples_t samples, size_t n);
  virtual void genRawData(uint8_t *rawdata, size_t n);

};

class ToneGen : public Generator
{
public:
  ToneGen(): phase(0), freq(0) {}
  ToneGen(Speakers spk, int freq, double phase, uint64_t stream_len, size_t chunk_size = 4096):
  Generator(spk, stream_len, chunk_size), phase(0), freq(0)
  {
    init(spk, freq, phase, stream_len, chunk_size);
  }

  bool init(Speakers spk, int freq, double phase, uint64_t stream_len, size_t chunk_size = 4096);

protected:
  double phase;
  double freq;

  virtual bool querySpk(Speakers spk) const;
  virtual void genSamples(samples_t samples, size_t n);
  virtual void genRawData(uint8_t *rawdata, size_t n);

};

class LineGen : public Generator
{
public:
  LineGen(): phase(0), k(1.0) {}
  LineGen(Speakers spk, double start, double k, uint64_t stream_len, size_t chunk_size = 4096):
  Generator(spk, stream_len, chunk_size), phase(0), k(1.0)
  {
     init(spk, start, k, stream_len, chunk_size);
  }

  bool init(Speakers spk, double start, double k, uint64_t stream_len, size_t chunk_size = 4096);

protected:
  double phase;
  double k;

  virtual bool querySpk(Speakers spk) const;
  virtual void genSamples(samples_t samples, size_t n);
  virtual void genRawData(uint8_t *rawdata, size_t n);

};

}; // namespace AudioFilter

#endif

// vim: ts=2 sts=2 et

