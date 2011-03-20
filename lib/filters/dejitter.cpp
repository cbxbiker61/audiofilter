#include <math.h>
#include <string.h>
#include "dejitter.h"

// uncomment this to log timing information into DirectShow log
//#define SYNCER_LOG_TIMING

#ifdef SYNCER_LOG_TIMING
#include <streams.h>
#endif

static const int format_mask_dejitter = FORMAT_CLASS_PCM | FORMAT_MASK_LINEAR | FORMAT_MASK_SPDIF;

///////////////////////////////////////////////////////////
// SyncerStat

Syncer::SyncerStat::SyncerStat()
{
  reset();
}

void   
Syncer::SyncerStat::reset()
{
  memset(stat, 0, sizeof(stat));
}

void   
Syncer::SyncerStat::add(vtime_t _val)
{
  memmove(stat + 1, stat, sizeof(stat) - sizeof(stat[0]));
  stat[0] = _val;
}

vtime_t 
Syncer::SyncerStat::stddev() const
{
  vtime_t sum = 0;
  vtime_t avg = mean();
  for (int i = 0; i < array_size(stat); i++)
    sum += (stat[i] - avg) * (stat[i] - avg);
  return sqrt(sum/array_size(stat));
}

vtime_t 
Syncer::SyncerStat::mean() const
{
  vtime_t sum = 0;
  for (int i = 0; i < array_size(stat); i++)
    sum += stat[i];
  return sum/array_size(stat);
}

int 
Syncer::SyncerStat::len() const
{
  return array_size(stat);
}

///////////////////////////////////////////////////////////
// Filter interface

bool
Syncer::query_input(Speakers _spk) const
{
  if (!_spk.sample_rate)
    return false;

  return (FORMAT_MASK(_spk.format) & format_mask_dejitter) != 0;
}

bool
Syncer::set_input(Speakers _spk)
{
  reset();

  if (!_spk.sample_rate)
    return false;

  switch (_spk.format)
  {
    case FORMAT_LINEAR:
      size2time = 1.0 / _spk.sample_rate;
      break;

    case FORMAT_PCM16:
    case FORMAT_PCM16_BE:
      size2time = 1.0 / 2.0 / _spk.nch()  / _spk.sample_rate;
      break;

    case FORMAT_PCM24:
    case FORMAT_PCM24_BE:
      size2time = 1.0 / 3.0 / _spk.nch()  / _spk.sample_rate; 
      break;

    case FORMAT_PCM32:
    case FORMAT_PCM32_BE:
      size2time = 1.0 / 4.0 / _spk.nch()  / _spk.sample_rate;
      break;

    case FORMAT_PCMFLOAT:
      size2time = 1.0 / sizeof(float) / _spk.nch()  / _spk.sample_rate;
      break;

    case FORMAT_PCMDOUBLE:
      size2time = 1.0 / sizeof(double) / _spk.nch()  / _spk.sample_rate;
      break;

    case FORMAT_SPDIF:
      size2time = 1.0 / 4.0 / _spk.sample_rate;
      break;

    default:
      return false;
  }

  return NullFilter::set_input(_spk);
}


void 
Syncer::reset()
{
  #ifdef SYNCER_LOG_TIMING
    DbgLog((LOG_TRACE, 3, "sync drop"));
  #endif
  continuous_sync = false;
  continuous_time = 0.0;
  istat.reset();
  ostat.reset();
  NullFilter::reset();
}

bool 
Syncer::process(const Chunk *_chunk)
{
  // we must ignore dummy chunks
  if (_chunk->is_dummy())
    return true;

  // receive chunk
  FILTER_SAFE(receive_chunk(_chunk));

  // ignore non-sync chunks
  if (!_chunk->sync)
    return true;

  // catch syncronization
  if (!continuous_sync)
  {
    #ifdef SYNCER_LOG_TIMING
      DbgLog((LOG_TRACE, 3, "sync catch: %ims", int(time * 1000)));
    #endif
    continuous_sync = true;
    continuous_time = time;
    return true;
  }

  // do dejitter
  vtime_t delta = time - continuous_time;

  if (dejitter)
  {
    // If received time stamp is too far from continuous 
    // time scale we must change our mind and change 
    // the time scale.
    //
    // This metrhod provides coarse syncronization (more 
    // than packet length, threshold should be 50-200ms)
    // and should be used only when fine syncronization
    // does not work or is not applicable.

    if (fabs(delta) > threshold)
    {
      #ifdef SYNCER_LOG_TIMING
        DbgLog((LOG_TRACE, 3, "sync lost; resync: %ims", int(time * 1000)));
      #endif
      continuous_sync = true;
      continuous_time = time;
      istat.reset();
      ostat.reset();
      return true;
    }

    // If time delta average is not null it means that
    // continuous time drift or single time shift is
    // present. In this case we should slowly compensate
    // the difference.
    //
    // Time drift may appear because of instability of
    // audio card clock for live-captured video. Typical
    // difference is about several seconds per hour. It is
    // noticable and very annoying difference so we must 
    // try to compensate it.
    //
    // Time drift may be described as linear time scale
    // transform:
    //   t' = t * k + c
    // where
    //   t' - real time scale
    //   t  - continious time scale based on sample rate
    //   k  - clock difference
    //
    // Problem is that k that describes difference between
    // time scales is unknown, and we cannot just follow 
    // real time scale (because we're trying to maintain
    // continuous time scale). So we must measure it.
    //
    // Suppose that at initial time t[0] = 0 we was in sync
    // (c = 0) and timestamps are distributed uniformly:
    //   t[n+1] = t[n] + dt (dt = const)
    //
    // In this case:
    //   t'[n+1] = k * (t[n] + dt)
    //
    //   delta[n] = t'[n] - t[n]
    //   delta[n+1] = t'[n+1] - t[n+1]
    //   delta[n+1] = delta[n] + dt * (k-1) = delta[n] + corr
    //
    //   delta[0] = 0
    //   delta[n] = n * corr
    //
    // where
    //   delta[n] - difference between continuous and real
    //              time scales.
    //   corr     - correction have to be applied (see below)
    //
    // To compensate difference between time scales we should
    // apply correction to each time stamp to maintain zero 
    // difference between time stamps (delta[n] = 0). This
    // correction value is 'corr'. We do not know k and dt but
    // we can measure mean delta value:
    //   mean = mean(delta[n]) = sum(delta[n]) / N
    //   mean = corr * N / 2
    //
    // From where we can find the correction to be applied:
    //   corr = mean * 2 / N
    //
    // All that was said before is also applicable when
    // jitter is present at input time stamps (when input
    // time stamps contain random error).
    //
    // But by applying the correction we introduce positive
    // feedback that may produce self-excited oscillation.
    // To prevent this we do not apply correction if mean
    // value is less than standard deviation. This allows
    // to suppress oscillation and keep timestamps at
    // confidence interval.
    //
    // This method provides fine syncronization. Difference
    // between smoothed time scale and real time scale is
    // guarantied to be at confidence interval. This 
    // interval depends on jitter level, that in order 
    // depends on number of samples per frame and sample 
    // rate:
    //
    // format | samples/frame | jitter stddev
    // -------+---------------+--------------
    //  mpa   |     1152      |      12 ms
    //  ac3   |     1536      |       9 ms
    //  dts   |   <= 16384    |   < 100 ms
    //
    // Threshold value for coarse syncronization should be
    // larger than jitter level (100 ms by default is 
    // reasonable enough).

    vtime_t mean = istat.mean();
    vtime_t stddev = istat.stddev();
    vtime_t correction = 0;

    if (fabs(mean) > stddev)
    {
      correction = istat.mean() * 2 / istat.len();
      continuous_time += correction;
    }

    istat.add(delta);
    ostat.add(correction);

    #ifdef SYNCER_LOG_TIMING
      DbgLog((LOG_TRACE, 3, "input:  %-6.0f delta: %-6.0f stddev: %-6.0f mean: %-6.0f", time, delta, istat.stddev(), istat.mean()));
      DbgLog((LOG_TRACE, 3, "output: %-6.0f correction: %-6.0f", continuous_time, correction));
    #endif
  }
  else // no dejitter
  {
    istat.add(delta);
    ostat.add(delta);

    #ifdef SYNCER_LOG_TIMING
      DbgLog((LOG_TRACE, 3, "input:  %-6.0f delta: %-6.0f stddev: %-6.0f mean: %-6.0f", time, delta, istat.stddev(), istat.mean()));
    #endif
  }

  return true;
}

bool 
Syncer::get_chunk(Chunk *_chunk)
{
  if (dejitter)
  {
    sync = continuous_sync;
    time = continuous_time * time_factor + time_shift;
  }
  else if (sync)
    time = time * time_factor + time_shift;

  continuous_time += size * size2time;
  send_chunk_inplace(_chunk, size);
  sync = false;
  return true;
}
