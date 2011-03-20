/*
  Levels & Histogram classes and filter
  Reports about current audio levels (with syncronization with ext. clock)
  Levels histogram 

  Speakers: unchanged
  Input formats: Linear
  Buffering: no
  Timing: unchanged
  Parameters:
    nsamples - number of samples for averaging
    dbpb     - dB per bin (for level histogram)
    levels   - levels
*/


#ifndef VALIB_LEVELS_H
#define VALIB_LEVELS_H

#include <string.h>
#include "../filter.h"

class LevelsCache;
class LevelsHistogram;
class Levels;


#define MAX_LEVELS_CACHE 256
#define MAX_HISTOGRAM    128

///////////////////////////////////////////////////////////////////////////////
// LevelsCache calss
///////////////////////////////////////////////////////////////////////////////

class LevelsCache
{
protected:
  sample_t levels_cache[MAX_LEVELS_CACHE][NCHANNELS];
  vtime_t  levels_time[MAX_LEVELS_CACHE];

  int pos;
  int end;

  inline int next_pos(int p);
  inline int prev_pos(int p);

public:
  LevelsCache();

  void reset();
  void add_levels(vtime_t time, sample_t levels[NCHANNELS]);
  void get_levels(vtime_t time, sample_t levels[NCHANNELS], bool drop = true);
};

///////////////////////////////////////////////////////////////////////////////
// LevelsHistogram calss
///////////////////////////////////////////////////////////////////////////////

class LevelsHistogram
{
protected:
  sample_t max_level[NCHANNELS];
  int histogram[NCHANNELS][MAX_HISTOGRAM];
  int n;
  int dbpb; // dB per bin

public:
  LevelsHistogram(int _dbpb = 5);

  void reset();

  int  get_dbpb() const;
  void set_dbpb(int dbpb);

  void add_levels(sample_t levels[NCHANNELS]);
  void get_histogram(double *histogram, size_t count) const;
  void get_histogram(int ch, double *histogram, size_t count) const;

  sample_t get_max_level() const;
  sample_t get_max_level(int ch) const;
};

///////////////////////////////////////////////////////////////////////////////
// Levels filter calss
///////////////////////////////////////////////////////////////////////////////

class Levels : public NullFilter
{
protected:
  LevelsCache cache;
  LevelsHistogram hist;

  sample_t levels[NCHANNELS]; // currently filling 

  size_t nsamples; // number of samples per measure block
  size_t sample;   // current sample
  vtime_t continuous_time; // we need continuous time counter
 
  /////////////////////////////////////////////////////////
  // NullFilter overrides

  virtual void on_reset();
  virtual bool on_process();

public:
  Levels(size_t _nsamples = 1024, int _dbpb = 5)
  :NullFilter(FORMAT_MASK_LINEAR)
  {
    set_nsamples(_nsamples);
    set_dbpb(_dbpb);
    reset();
  }

  /////////////////////////////////////////////////////////
  // Levels interface

  inline size_t get_nsamples() const;
  inline void set_nsamples(size_t);

  inline int  get_dbpb() const;
  inline void set_dbpb(int dbpb);

  inline void add_levels(vtime_t time, sample_t levels[NCHANNELS]);
  inline void get_levels(vtime_t time, sample_t levels[NCHANNELS], bool drop = true);
  inline void get_histogram(double *histogram, size_t count) const;
  inline void get_histogram(int ch, double *histogram, size_t count) const;
  inline sample_t get_max_level() const;
  inline sample_t get_max_level(int ch) const;
};

///////////////////////////////////////////////////////////
// Levels inlines

size_t
Levels::get_nsamples() const
{
  return nsamples;
}

void
Levels::set_nsamples(size_t _nsamples)
{
  nsamples = _nsamples;
}

int 
Levels::get_dbpb() const
{
  return hist.get_dbpb();
}

void 
Levels::set_dbpb(int _dbpb)
{
  hist.set_dbpb(_dbpb);
}

void 
Levels::add_levels(vtime_t _time, sample_t _levels[NCHANNELS])
{
  cache.add_levels(_time, _levels);
  hist.add_levels(_levels);
}

void 
Levels::get_levels(vtime_t _time, sample_t _levels[NCHANNELS], bool _drop)
{
  cache.get_levels(_time, _levels, _drop);
}

void 
Levels::get_histogram(double *_histogram, size_t _count) const
{
  hist.get_histogram(_histogram, _count);
}

void 
Levels::get_histogram(int _ch, double *_histogram, size_t _count) const
{
  hist.get_histogram(_ch, _histogram, _count);
}

sample_t
Levels::get_max_level() const
{
  return hist.get_max_level();
}

sample_t
Levels::get_max_level(int ch) const
{
  return hist.get_max_level(ch);
}

#endif
