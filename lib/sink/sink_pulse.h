/*
  PulseSink
  ==========

  Pulse audio renderer
  Implements PlaybackControl and Sink interfaces.

  PlaybackContol interface must be thread-safe.

  Blocking functions (process() and flush()) cannot take DirectSound lock for a
  long time because it may block the control thread and lead to a deadlock.
  Therefore to serialize playback functions 'playback_lock' is used.

  stop() must force blocking functions to finish. To signal these functions to
  unblock 'ev_stop' is used. Blocking functions must wait on this event and
  stop execution immediately when signaled.
*/

#ifndef VALIB_SINK_PULSE_H
#define VALIB_SINK_PULSE_H

#include <pulse/pulsaudio.h>
#include <mmreg.h>
#include <ks.h>
#include <ksmedia.h>
#include "../filter.h"
#include "../renderer.h"
#include "../win32/thread.h"

class PulseSink : public Sink, public PlaybackControl
{
public:
  PulseSink();
  ~PulseSink();

  /////////////////////////////////////////////////////////
  // Own interface

  bool openPulse(int buf_size_ms = 2000, int preload_ms = 500);
  //bool openPulse(HWND _hwnd, int buf_size_ms = 2000, int preload_ms = 500, LPCGUID device = 0);
  void closePulse(void);

  bool open(Speakers spk);
  void close(void);

  /////////////////////////////////////////////////////////
  // Playback control

  virtual void pause(void);
  virtual void unpause(void);
  virtual bool isPaused(void) const;

  virtual void stop(void);
  virtual void flush(void);

  virtual vtime_t getPlaybackTime(void) const;

  virtual size_t  getBufferSize(void) const;
  virtual vtime_t getBufferTime(void) const;
  virtual size_t  getDataSize(void) const;
  virtual vtime_t getDataTime(void) const;

#if 0
  virtual double getVol(void) const;
  virtual void   setVol(double vol);
  virtual double getPan(void) const;
  virtual void   setPan(double pan);
#endif

  /////////////////////////////////////////////////////////
  // Sink interface

  virtual bool queryNnput(Speakers _spk) const;
  virtual bool setInput(Speakers _spk);
  virtual Speakers getInput(void) const;
  virtual bool process(const Chunk *_chunk);

protected:
  int      buf_size_ms;   // buffer size in ms
  int      preload_ms;    // preload size in ms

  /////////////////////////////////////////////////////////
  // Parameters that depend on initialization
  // (do not change on processing)

  Speakers spk;           // playback format
  DWORD    buf_size;      // buffer size in bytes
  DWORD    preload_size;  // preload size in bytes
  double   bytes2time;    // factor to convert bytes to seconds

  pa_mainloop *_mainloop;
  pa_mainloop_api *_mainloop_api;
  pa_context *_context;
  pa_stream *_stream;

  /////////////////////////////////////////////////////////
  // Processing parameters

  DWORD    cur;           // cursor in sound buffer
  vtime_t  time;          // time of last sample received
  bool     playing;       // playing state
  bool     paused;        // paused state

  /////////////////////////////////////////////////////////
  // Threading

  //mutable CritSec dsound_lock;
  //mutable CritSec playback_lock;
  //HANDLE  ev_stop;

  /////////////////////////////////////////////////////////
  // Resource allocation

  bool open(WAVEFORMATEX *wfx);
  bool tryOpen(Speakers spk) const;
  bool tryOpen(WAVEFORMATEX *wf) const;

private:
  void quit(int ret);
};

#endif

