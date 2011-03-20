/*
  Jitter correction
  See valib/doc/jitter.xls for explanations
*/

#ifndef VALIB_DEJITTER_H
#define VALIB_DEJITTER_H

#include "../filter.h"

#define STAT_SIZE 64

class Syncer : public NullFilter
{
protected:
  double size2time;

  // continious time
  bool    continuous_sync;
  vtime_t continuous_time;

  // linear time transform
  vtime_t time_shift;
  vtime_t time_factor;

  // jitter correction
  bool    dejitter;
  vtime_t threshold;

  // statistics
  class SyncerStat
  {
  protected:
    vtime_t stat[STAT_SIZE];

  public:
    SyncerStat();

    void reset();
    void add(vtime_t);
    vtime_t stddev() const;
    vtime_t mean() const;
    int len() const;
  };
  SyncerStat istat;
  SyncerStat ostat;

public:
  Syncer()
  :NullFilter(FORMAT_MASK_LINEAR | FORMAT_CLASS_PCM | FORMAT_MASK_SPDIF)
  {
    size2time   = 1.0;

    continuous_sync = false;
    continuous_time = 0.0;

    time_shift  = 0;
    time_factor = 1.0;

    dejitter  = true;
    threshold = 0.1;
  }

  /////////////////////////////////////////////////////////
  // Syncer interface

  void    resync()                               { sync = false; }

  // Linear time transform
  vtime_t get_time_shift() const                 { return time_shift; }
  void    set_time_shift(vtime_t _time_shift)    { time_shift = _time_shift; }

  vtime_t get_time_factor() const                { return time_factor; }
  void    set_time_factor(vtime_t _time_factor ) { time_factor = _time_factor; }

  // Jitter
  bool    get_dejitter() const                   { return dejitter; }
  void    set_dejitter(bool _dejitter)           { dejitter = _dejitter; }

  vtime_t get_threshold() const                  { return threshold; }
  void    set_threshold(vtime_t _threshold)      { threshold = _threshold; }

  vtime_t get_input_mean() const                 { return istat.mean(); }
  vtime_t get_input_stddev() const               { return istat.stddev(); }
  vtime_t get_output_mean() const                { return ostat.mean(); }
  vtime_t get_output_stddev() const              { return ostat.stddev(); }

  /////////////////////////////////////////////////////////
  // Filter interface

  void reset();

  bool query_input(Speakers spk) const;
  bool set_input(Speakers spk);
  bool process(const Chunk *_chunk);

  bool get_chunk(Chunk *_chunk);
};

#endif
