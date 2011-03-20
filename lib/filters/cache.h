/*
  Cache data to synchronize audio visualization with actual playback

  Due to buffering done by playback device, audio visualization is not
  synchronized with audio playback. To show current activity we should access
  data back in time. This filter buffers audio data and allows to access
  the moment we currently play. 
*/

#ifndef VALIB_CACHE_H
#define VALIB_CACHE_H

#include "linear_filter.h"
#include "../buffer.h"

class CacheFilter : public LinearFilter
{
protected:
  vtime_t stream_time; // Time after the last cached sample
  SampleBuf buf;       // Cache buffer (circular buffer)

  vtime_t buf_size;    // Size of the buffer in time units
  int buf_samples;     // Size fo the buffer in samples
  int cached_samples;  // Number of samples cached
  int pos;             // Position of the end of the circular buffer

  virtual bool init(Speakers spk, Speakers &out_spk);
  virtual void reset_state();
  virtual void sync(vtime_t time);
  virtual bool process_inplace(samples_t in, size_t in_size);

public:
  CacheFilter();
  CacheFilter(vtime_t size);

  vtime_t get_time() const;
  vtime_t get_size() const;
  void set_size(vtime_t size);

  // Copy data cached into the buffer
  // You can specify CH_NONE to get sum of all channels
  // Returns actual number of samples copied
  size_t get_samples(int ch_name, vtime_t time, sample_t *buf, size_t size);
};

#endif
