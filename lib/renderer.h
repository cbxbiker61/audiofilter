#ifndef VALIB_RENEDERER_H
#define VALIB_RENEDERER_H

#include "vtime.h"

///////////////////////////////////////////////////////////////////////////////
// Interface to control audio playback.
//
// This interface is proposed to control audio playback. Implement it if you
// do audio output and want to control it. Primary purpose is to use it in
// conjunction with Sink interface in objects that implements audio playback.
//
// Note that PlaybackControl functions may be called from both control and
// working threads when Sink functions can be called only from working thread.
// Threrfore PlaybackControl functions must be thread-safe.
//
// Playback control functions
// ==========================
//
// pause() [non-blocking]
//   Pause audio playback. Do not close anything, just stop the playback. This
//   call may block working thread if called from another thread. It must not
//   block the thread it was called from therefore it may be also called from
//   the working thread.
//
//   It may be called when playback was not actually started. In this case
//   working thread should fill output buffer and stop without actually
//   starting the playback. unpause() will actually start playback in this
//   case.
//
//   If pausing is not supported is_paused() must report unpaused state after
//   this call.
//
// unpause() [non-blocking]
//   Continue paused playback. Do nothing if playback was not paused.
//
// is_paused() [non-blocking]
//   Report about current pause status. If pausing is not supported it must
//   report unpaused state after pause().
//
// stop() [non-blocking]
//   Immediately stop playback and drop buffered data. Primary purpose of this
//   function is to drop buffered data and prepare to continue playback of a
//   new stream and/or from a new position. This function shuold not actually
//   close audio output, just stop playback.
//
//   All functions blocked on plyback (flush(), Sink::process()) must unblock
//   and return normally (it is not an error condition) without processing of
//   data remaining to process.
//
//   Sink::set_input() may use this function to stop current playback before
//   opening of a new stream.
//
// flush() [blocking]
//   Wait until all buffered data is played and stop playback. Purpose of this
//   function is to finish playback correctly without loosing buffered tail.
//   This function blocks until the end of playback. This function may use
//   stop() to finish audio playback. Sink::process() may use this function
//   after receiving eos-chunk.
//
//   This function must exit immediately on stop() call.
//
// Timing and buffering
// ====================
//
// get_playback_time()
//   Reports about current playback time (position of current playback cursor)
//   in the stream. It should take in account timestamps received with chunks
//   at Sink::process().
//
// get_buffer_size()
// get_buffer_time()
//   Report buffer size in bytes and time units. Must return 0 when object
//   does not do buffering or buffer size is unknown.
//
// get_data_size()
// get_data_time()
//   Report size of buffered data in bytes and time units. Must return 0 when
//   object does not do buffering or buffered data size is unknown.
//
// Volume and pan controls
// ===========================
//
// get_vol()
// set_vol()
//   Control current output volume. Volume is interpreted as amplification
//   in dB. Therefore 0 is maximum level (no amplification) negative values
//   represent levels less than maximum and positiva valuse represent
//   over-maximum amplification (generally unsupported by most APIs).
//   set_vol() should check the volume value and limit it to supported range.
//   get_vol() reports actual volume level. Therefore if level control is
//   unsupported it should always return 0.
//
// get_pan()
// set_pan()
//   Control panning. Panning value is interpreted as elvel difference between
//   left and right channels in dB. Therefore 0 is no panning, positive values
//   means that left channel(s) is louder, negative values means that right
//   channle(s) is louder
//   set_pan() should check the panning value and limit it to supported range.
//   get_pan() reports actual panning level. Therefore if pan ontrol is
//   unsupported it should always return 0.


class PlaybackControl
{
public:
  PlaybackControl() {}
  virtual ~PlaybackControl() {}

  virtual void pause(void) {}
  virtual void unpause(void) {}

  virtual bool isPaused(void) const
  {
    return false;
  }

  virtual void stop(void) {}
  virtual void flush(void) {}

  virtual vtime_t getPlaybackTime(void) const
  {
    return 0;
  }

  virtual size_t getBufferSize(void) const
  {
    return 0;
  }

  virtual vtime_t getBufferTime(void) const
  {
    return 0;
  }

  virtual size_t getDataSize(void) const
  {
    return 0;
  }

  virtual vtime_t getDataTime(void) const
  {
    return 0;
  }

  virtual double getVol(void) const
  {
    return 0.0;
  }

  virtual void setVol(double vol) {}

  virtual double getPan(void) const
  {
    return 0;
  }

  virtual void setPan(double pan) {}
};

#endif

