#include <cstring>
#include <math.h>
#include "AgcFilter.h"

#define LEVEL_MINUS_50DB 0.0031622776601683793319988935444327
#define LEVEL_MINUS_100DB 0.00001
#define LEVEL_PLUS_100DB 100000.0

namespace AudioFilter {

AgcFilter::AgcFilter(size_t _nsamples)
  : NullFilter(FORMAT_MASK_LINEAR)
{
  block       = 0;

  sample[0]  = 0;
  sample[1]  = 0;
  buf_sync[0] = false;
  buf_sync[1] = false;
  buf_time[0] = 0;
  buf_time[1] = 0;

  nsamples  = 0;

  level     = 0;
  factor    = 0;

  // Options
  auto_gain = true;
  normalize = false;

  // Gain control
  master    = 1.0;   // factor
  gain      = 1.0;   // factor
  attack    = 50.0;  // dB/s
  release   = 50.0;  // dB/s

  // DRC
  drc       = false;
  drc_power = 0;     // dB; this value has meaning of loudness raise at -50dB level
  drc_level = 1.0;   // factor

  // rebuild window
  setBuffer(_nsamples);
}

void AgcFilter::setBuffer(size_t _nsamples)
{
  // allocate buffers
  nsamples = _nsamples;
  buf[0].allocate(NCHANNELS, nsamples);
  buf[1].allocate(NCHANNELS, nsamples);
  w.allocate(2, nsamples);

  // hann window
  double f = 2.0 * M_PI / (nsamples * 2);

  for ( size_t i = 0; i < nsamples; ++i )
  {
    w[0][i] = 0.5 * (1 - cos(i*f));
    w[1][i] = 0.5 * (1 - cos((i+nsamples)*f));
  }

  // reset
  reset();
}

size_t AgcFilter::getBuffer(void) const
{
  return nsamples;
}

bool AgcFilter::fillBuffer(void)
{
  if ( sync && sample[block] == 0 )
  {
    buf_sync[block] = sync;
    buf_time[block] = time;
    sync = false;
  }

  size_t n(nsamples - sample[block]);

  const int maxCh(spk.getChannelCount());

  if ( size < n )
  {
    for ( int ch = 0; ch < maxCh; ++ch )
      memcpy(buf[block][ch] + sample[block], samples[ch], size * sizeof(sample_t));

    sample[block] += size;
    time += vtime_t(size) / spk.getSampleRate();
    dropSamples(size);

    if ( ! flushing )
      return false;

    // zero rest of buffer in case of flushing
    for ( int ch = 0; ch < maxCh; ++ch )
      memset(buf[block][ch] + sample[block], 0, (nsamples - sample[block]) * sizeof(sample_t));

    return true;
  }
  else
  {
    for ( int ch = 0; ch < maxCh; ++ch )
      memcpy(buf[block][ch] + sample[block], samples[ch], n * sizeof(sample_t));

    sample[block] = nsamples;
    time += vtime_t(n) / spk.getSampleRate();
    dropSamples(n);

    return true;
  }
}

void AgcFilter::process(void)
{
  const int maxCh(spk.getChannelCount());
  const sample_t spk_level(spk.getLevel());

  sample_t max;
  sample_t *sptr;
  sample_t *send;

  sample_t levels_loc[NCHANNELS];
  memset(levels_loc, 0, sizeof(levels_loc));

  ///////////////////////////////////////
  // Channel levels

  for ( int ch = 0; ch < maxCh; ++ch )
  {
    max = 0;
    sptr = buf[block][ch];
    send = sptr + nsamples - 7;

    while ( sptr < send )
    {
      if ( fabs(sptr[0]) > max )
        max = fabs(sptr[0]);
      if ( fabs(sptr[1]) > max )
        max = fabs(sptr[1]);
      if ( fabs(sptr[2]) > max )
        max = fabs(sptr[2]);
      if ( fabs(sptr[3]) > max )
        max = fabs(sptr[3]);
      if ( fabs(sptr[4]) > max )
        max = fabs(sptr[4]);
      if ( fabs(sptr[5]) > max )
        max = fabs(sptr[5]);
      if ( fabs(sptr[6]) > max )
        max = fabs(sptr[6]);
      if ( fabs(sptr[7]) > max )
        max = fabs(sptr[7]);

      sptr += 8;
    }

    send += 7;

    while ( sptr < send )
    {
      if ( fabs(sptr[0]) > max )
        max = fabs(sptr[0]);

      ++sptr;
    }

    levels_loc[ch] = max / spk_level;
  }

  ///////////////////////////////////////
  // Gain, Limiter, DRC

  sample_t old_factor = factor;
  sample_t old_level = level;
  sample_t release_factor;
  sample_t attack_factor;

  // attack/release factor
  if ( attack < 0 )
    attack  = 0;

  if ( release < 0 )
    release = 0;

  attack_factor  = pow(10.0, attack  * nsamples / spk.getSampleRate() / 20);
  release_factor = pow(10.0, release * nsamples / spk.getSampleRate() / 20);

  // block level

  level = levels_loc[0];

  for ( int ch = 1; ch < maxCh; ++ch )
  {
    if ( level < levels_loc[ch] )
      level = levels_loc[ch];
  }

  // Here 'level' is block peak-level. Normally, this level should not be
  // greater than 1.0 (0dB) but it is possible that some post-processing made
  // it larger. Our task is to decrease the global gain to make the output
  // level <= 1.0.

  // adjust gain (release)

  if ( ! auto_gain )
    gain = master;
  else if ( ! normalize )
  {
    if ( gain * release_factor > master )
      gain = master;
    else
      gain *= release_factor;
  }

  // DRC

  if ( drc )
  {
    sample_t compressed_level;

    if ( level > LEVEL_MINUS_50DB )
      compressed_level = pow(level, -drc_power/50.0);
    else
      compressed_level = pow(level * LEVEL_PLUS_100DB, drc_power/50.0);

    sample_t released_level = drc_level * release_factor;

    if ( level < LEVEL_MINUS_100DB )
      drc_level = 1.0;
    else if ( released_level > compressed_level )
      drc_level = compressed_level;
    else
      drc_level = released_level;
  }
  else
    drc_level = 1.0;

  // factor

  factor = gain * drc_level;

  // adjust gain on overflow

  max = MAX(level, old_level) * factor;

  if ( auto_gain && max > 1.0 )
  {
    if ( max < attack_factor )
    {
      // corrected with no overflow
      factor /= max;
      gain   /= max;
      max     = 1.0;
    }
    else
    {
      // overflow, will be clipped
      factor /= attack_factor;
      gain   /= attack_factor;
      max    /= attack_factor;
    }
  }

  ///////////////////////////////////////
  // Switch blocks

  block  = getNextBlock();

  ///////////////////////////////////////
  // Windowing
  //
  // * full windowing on gain change
  // * simple gain when gain is applied
  // * no windowing if it is no gain applied

  if ( ! EQUAL_SAMPLES(old_factor, factor) )
  {
    // windowing
    for ( int ch = 0; ch < maxCh; ++ch )
    {
      sptr = buf[block][ch];

      for ( size_t s = 0; s < nsamples; ++s, ++sptr )
      {
        *sptr = *sptr * old_factor * w[1][s]
                + *sptr * factor * w[0][s];
      }
    }
  }
  else if ( ! EQUAL_SAMPLES(factor, 1.0) )
  {
    // simple gain
    for ( int ch = 0; ch < maxCh; ++ch )
    {
      sptr = buf[block][ch];

      for ( size_t s = 0; s < nsamples; ++s, ++sptr )
        *sptr *= factor;
    }
  }

  ///////////////////////////////////////
  // Clipping
  // Note that we must clip even in case
  // of previous block overflow...

  if ( level * factor > 1.0 || old_level * old_factor > 1.0 )
  {
    for ( int ch = 0; ch < maxCh; ++ch )
    {
      sptr = buf[block][ch];

      for ( size_t s = 0; s < nsamples; ++s, ++sptr )
      {
        if ( *sptr > +spk_level )
          *sptr = +spk_level;
        else if ( *sptr < -spk_level )
          *sptr = -spk_level;
      }
    }
  }
}

///////////////////////////////////////////////////////////
// Filter interface

void AgcFilter::reset(void)
{
  NullFilter::reset();

  block = 0;

  sample[0] = 0;
  sample[1] = 0;
  buf_sync[0] = false;
  buf_sync[1] = false;
  buf_time[0] = 0;
  buf_time[1] = 0;

  level = 1.0;
  factor= 1.0;
}

bool AgcFilter::getChunk(Chunk *_chunk)
{
  while ( fillBuffer() )
  {
    process();

    // do not send empty block (first block)
    if ( ! sample[block] && sample[getNextBlock()] )
      continue;

    // send data
    _chunk->setLinear( spk, buf[block], sample[block]
                      , buf_sync[block], buf_time[block]
                      , flushing && (sample[getNextBlock()] == 0) );
    buf_sync[block] = false;
    sample[block] = 0; // drop block just sent
    flushing = flushing && (sample[getNextBlock()] != 0); // drop flushing state
    return true;
  }

  // assert: size == 0
  // no more input data left to process

  _chunk->setDummy();
  return true;
}

}; // namespace AudioFilter

