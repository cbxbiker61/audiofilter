#include <math.h>
#include "sink_dsound.h"
#include "../win32/winspk.h"

#define SAFE_RELEASE(p) { if (p) p->Release(); p = 0; }


PulseSink::PulseSink()
  : buf_size_ms(0)
  , preload_ms(0)
  , spk(spk_unknown)
  , buf_size(0)
  , preload_size(0)
  , bytes2time(0.0)
  , _mainloop(0)
  , _mainloop_api(0)
  , _context(0)
  , _stream(0)
  , cur(0)
  , time(0)
  , playing(false)
  , paused(true)
{
}

PulseSink::~PulseSink()
{
  close();
  closePulse();
}

///////////////////////////////////////////////////////////////////////////////
// Resource allocation

bool
PulseSink::openPulse(int _buf_size_ms, int _preload_ms)
{
  if ( _mainloop )
    closePulse();

  if ( ! (_mainloop = pa_mainloop_new()) )
  {
    fprintf(stderr, "pa_mainloop_new() failed.");
    return false;
  }

  if ( ! (_mainloop_api = pa_mainloop_get_api(_mainloop)) )
  {
    fprintf(stderr, "pa_mainloop_get_api() failed.");
    return false;
  }

  /* Create a new connection context */
  if ( ! (_context = pa_context_new_with_proplist(_mainloop_api, 0, proplist)) )
  {
    fprintf(stderr, "pa_context_new() failed.\n");
    return false;
  }

  buf_size_ms = _buf_size_ms;
  preload_ms = _preload_ms;
  return true;

quit:
    if (stream)
        pa_stream_unref(stream);

    if ( _context )
        pa_context_unref(context);

    if (stdio_event) {
        pa_assert(mainloop_api);
        mainloop_api->io_free(stdio_event);
    }

    if (time_event) {
        pa_assert(mainloop_api);
        mainloop_api->time_free(time_event);
    }

    if (m) {
        pa_signal_done();
        pa_mainloop_free(m);
    }

    pa_xfree(buffer);

    pa_xfree(server);
    pa_xfree(device);

    if (sndfile)
    {
#if 1
        fclose(sndfile);
#else
        sf_close(sndfile);
#endif
    }

    if (proplist)
        pa_proplist_free(proplist);

    return ret;
}

void PulseSink::quitMainloopApi(int ret)
{
  if ( _mainloop_api )
  {
    _mainloop_api->quit(mainloop_api, ret);
    _mainloop_api = 0;
  }
}

/* UNIX signal to quit recieved */
static void exit_signal_callback(pa_mainloop_api*m
  , pa_signal_event *e, int sig, void *userdata)
{
  quit(0);
}

void
PulseSink::closePulse(void)
{
  close();

  if ( _stream )
  {
    pa_stream_unref(_stream);
    _stream = 0;
  }

  if ( _context )
  {
    pa_context_unref(_context);
    _context = 0;
  }

#if 0
  if ( _stdio_event )
  {
    pa_assert(mainloop_api);
    mainloop_api->io_free(_stdio_event);
  }

  if ( time_event )
  {
    pa_assert(mainloop_api);
    mainloop_api->time_free(_time_event);
  }
#endif

  quit(0);

  if ( _mainloop )
  {
    pa_signal_done();
    pa_mainloop_free(_mainloop);
    _mainloop = 0;
  }
}

bool
PulseSink::open(Speakers _spk)
{
  if ( ! ds )
    return false;

  AutoLock autolock(&dsound_lock);

  WAVEFORMATEXTENSIBLE wfx;
  memset(&wfx, 0, sizeof(wfx));

  close();
  spk = _spk;

  if ( spk2wfx(_spk, (WAVEFORMATEX*)(&wfx), true)
      && open((WAVEFORMATEX*)(&wfx)))
    return true;

  if ( spk2wfx(_spk, (WAVEFORMATEX*)&wfx, false)
      && open((WAVEFORMATEX*)(&wfx)))
    return true;

  close();
  return false;
}

bool
PulseSink::open(WAVEFORMATEX *wf)
{
  // This function is called only from open() and therefore
  // we do not take any lock here.

  buf_size = wf->nBlockAlign * wf->nSamplesPerSec * buf_size_ms / 1000;
  preload_size = wf->nBlockAlign * wf->nSamplesPerSec * preload_ms / 1000;
  bytes2time = 1.0 / wf->nAvgBytesPerSec;

  // DirectSound buffer description
  DSBUFFERDESC dsbdesc;
  memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
  dsbdesc.dwSize        = sizeof(DSBUFFERDESC);
  dsbdesc.dwFlags       = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
  dsbdesc.dwFlags      |= DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN;
  dsbdesc.dwBufferBytes = buf_size;
  dsbdesc.lpwfxFormat   = wf;

  // Try to create buffer with volume and pan controls
  if FAILED(ds->CreateSoundBuffer(&dsbdesc, &ds_buf, 0))
  {
    SAFE_RELEASE(ds_buf);

    // Try to create buffer without volume and pan controls
    dsbdesc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
    if FAILED(ds->CreateSoundBuffer(&dsbdesc, &ds_buf, 0))
    {
      SAFE_RELEASE(ds_buf);
      return false;
    }
  }

  // Zero playback buffer
  void *data;
  DWORD data_bytes;

  if FAILED(ds_buf->Lock(0, buf_size, &data, &data_bytes, 0, 0, 0))
  {
    SAFE_RELEASE(ds_buf);
    SAFE_RELEASE(ds);
    return false;
  }

  memset(data, 0, data_bytes);
  if FAILED(ds_buf->Unlock(data, data_bytes, 0, 0))
  {
    SAFE_RELEASE(ds_buf);
    SAFE_RELEASE(ds);
    return false;
  }

  // Prepare to playback
  ds_buf->SetCurrentPosition(0);
  cur = 0;
  time = 0;
  playing = false;
  paused = false;
  return true;
}

bool
PulseSink::tryOpen(Speakers _spk) const
{
  if ( ! ds )
    return false;

  WAVEFORMATEXTENSIBLE wfx;
  memset(&wfx, 0, sizeof(wfx));

  if ( spk2wfx(_spk, (WAVEFORMATEX*)(&wfx), true)
      && tryOpen((WAVEFORMATEX*)(&wfx)) )
    return true;

  if ( spk2wfx(_spk, (WAVEFORMATEX*)&wfx, false)
      && tryOpen((WAVEFORMATEX*)(&wfx)) )
    return true;

  return false;
}

bool
PulseSink::tryOpen(WAVEFORMATEX *wf) const
{
  // This function do not use shared DirectSound buffer and
  // therefore we do not take any lock here.

  IDirectSoundBuffer *test_ds_buf = 0;

  DWORD test_buf_size = wf->nBlockAlign * wf->nSamplesPerSec * buf_size_ms / 1000;

  // DirectSound buffer description
  DSBUFFERDESC dsbdesc;
  memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
  dsbdesc.dwSize        = sizeof(DSBUFFERDESC);
  dsbdesc.dwFlags       = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
  dsbdesc.dwFlags      |= DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN;
  dsbdesc.dwBufferBytes = test_buf_size;
  dsbdesc.lpwfxFormat   = wf;

  // Try to create buffer with volume and pan controls
  if FAILED(ds->CreateSoundBuffer(&dsbdesc, &test_ds_buf, 0))
  {
    SAFE_RELEASE(test_ds_buf);

    // Try to create buffer without volume and pan controls
    dsbdesc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
    if FAILED(ds->CreateSoundBuffer(&dsbdesc, &test_ds_buf, 0))
    {
      SAFE_RELEASE(test_ds_buf);
      return false;
    }
  }

  SAFE_RELEASE(test_ds_buf);
  return true;
}

void
PulseSink::close(void)
{
  AutoLock autolock(&dsound_lock);

  /////////////////////////////////////////////////////////
  // Unblock playback and take playback lock. It is safe
  // because ev_stop remains signaled (manual-reset event)
  // and playback functions cannot block anymore.

  AutoLock playback(&playback_lock);

  /////////////////////////////////////////////////////////
  // Close everything

  spk          = spk_unknown;
  buf_size     = 0;
  preload_size = 0;
  bytes2time   = 0.0;

  SAFE_RELEASE(ds_buf);

  cur     = 0;
  time    = 0;
  playing = false;
  paused  = false;
}

///////////////////////////////////////////////////////////////////////////////
// Own interface

///////////////////////////////////////////////////////////////////////////////
// Playback control

void
PulseSink::pause(void)
{
  AutoLock autolock(&dsound_lock);

  if ( ds_buf )
    ds_buf->Stop();

  paused = true;
}

void
PulseSink::unpause(void)
{
  AutoLock autolock(&dsound_lock);

  if ( ds_buf && playing )
    ds_buf->Play(0, 0, DSBPLAY_LOOPING);

  paused = false;
}

bool
PulseSink::isPaused(void) const
{
  return paused;
}

vtime_t
PulseSink::getPlaybackTime(void) const
{
  return time - getDataTime();
}

size_t
PulseSink::getBufferSize(void) const
{
  return buf_size;
}

vtime_t
PulseSink::getBufferTime(void) const
{
  return buf_size * bytes2time;
}

size_t
PulseSink::getDataSize(void) const
{
  if ( ! ds_buf ) return 0;
  AutoLock autolock(&dsound_lock);

  DWORD play_cur;
  ds_buf->GetCurrentPosition(&play_cur, 0);

  if (play_cur > cur)
    return buf_size + cur - play_cur;
  else if (play_cur < cur)
    return cur - play_cur;
  else
    // if playback cursor is equal to buffer write position it may mean:
    // * playback is stopped/paused so both pointers are equal (buffered size = 0)
    // * buffer is full so both pointers are equal (buffered size = buf_size)
    // * buffer underrun (we do not take this in account because in either case
    //   it produces playback glitch)
    return playing? buf_size: 0;
}

vtime_t
PulseSink::getDataTime(void) const
{
  return getDataSize() * bytes2time;
}

void
PulseSink::stop(void)
{
  AutoLock autolock(&dsound_lock);

  /////////////////////////////////////////////////////////
  // Unblock playback and take playback lock. It is safe
  // because ev_stop remains signaled (manual-reset event)
  // and playback functions cannot block anymore.
  AutoLock playback(&playback_lock);

  /////////////////////////////////////////////////////////
  // Stop and drop cursor positions

  if (ds_buf)
  {
    ds_buf->Stop();
    ds_buf->SetCurrentPosition(0);
  }

  cur = 0;
  time = 0;
  playing = false;
}

void
PulseSink::flush(void)
{
  /////////////////////////////////////////////////////////
  // We may not take output lock here because close() tries
  // to take playback lock before closing audio output.

  AutoLock autolock(&playback_lock);

  void *data1, *data2;
  DWORD data1_bytes, data2_bytes;
  DWORD play_cur;
  DWORD data_size;

  ///////////////////////////////////////////////////////
  // Determine size of data in playback buffer
  // data size - size of data to playback

  if FAILED(ds_buf->GetCurrentPosition(&play_cur, 0))
    return;

  if (cur < play_cur)
    data_size = buf_size + cur - play_cur;
  else if (cur > play_cur)
    data_size = cur - play_cur;
  else
  {
    if (!playing)
      // we have nothing to flush...
      return;
    else
      data_size = buf_size;
  }

  ///////////////////////////////////////////////////////
  // Sleep until we have half of playback buffer free
  // Note that we must finish immediately on ev_stop

#if 0
  if ( data_size > buf_size / 2 )
  {
    data_size -= buf_size / 2;
    if (WaitForSingleObject(ev_stop, DWORD(data_size * bytes2time * 1000 + 1)) == WAIT_OBJECT_0)
      return;
  }
#endif

  /////////////////////////////////////////////////////////
  // Zero the rest of the buffer
  // data size - size of data to zero

  if FAILED(ds_buf->GetCurrentPosition(&play_cur, 0))
    return;

  data_size = buf_size + play_cur - cur;
  if (data_size >= buf_size)
    data_size -= buf_size;

  if FAILED(ds_buf->Lock(cur, data_size, &data1, &data1_bytes, &data2, &data2_bytes, 0))
    return;

  memset(data1, 0, data1_bytes);
  if (data2_bytes)
    memset(data2, 0, data2_bytes);

  if FAILED(ds_buf->Unlock(data1, data1_bytes, data2, data2_bytes))
    return;

  /////////////////////////////////////////////////////////
  // Start playback if we're in prebuffering state

  if ( ! playing )
  {
    ds_buf->Play(0, 0, DSBPLAY_LOOPING);
    playing = true;
  }

  /////////////////////////////////////////////////////////
  // Sleep until the end of playback
  // Note that we must finish immediately on ev_stop
#if 0
  data_size = buf_size - data_size;
  if (WaitForSingleObject(ev_stop, DWORD(data_size * bytes2time * 1000 + 1)) == WAIT_OBJECT_0)
    return;
#endif

  /////////////////////////////////////////////////////////
  // Stop the playback

  stop();
}

#if 0
// Implement these functions when you get time
double
PulseSink::getVol(void) const
{
  if ( ! ds_buf ) return 0;
  AutoLock autolock(&dsound_lock);

  LONG vol;
  if SUCCEEDED(ds_buf->GetVolume(&vol))
    return (double)(vol / 100);
  return 0;
}

void
PulseSink::setVol(double vol)
{
  if ( ! ds_buf ) return;
  AutoLock autolock(&dsound_lock);
  ds_buf->SetVolume((LONG)(vol * 100));
}

double
PulseSink::getPan(void) const
{
  if (!ds_buf) return 0;
  AutoLock autolock(&dsound_lock);

  LONG pan;
  if SUCCEEDED(ds_buf->GetPan(&pan))
    return (double)(pan / 100);
  return 0;
}

void
PulseSink::setPan(double pan)
{
  if ( ! ds_buf ) return;
  AutoLock autolock(&dsound_lock);
  ds_buf->SetPan((LONG)(pan * 100));
}
#endif

///////////////////////////////////////////////////////////////////////////////
// Sink interface

bool
PulseSink::queryInput(Speakers _spk) const
{
  return tryOpen(_spk);
}

bool
PulseSink::setInput(Speakers _spk)
{
  AutoLock autolock(&dsound_lock);

  close();
  return open(_spk);
}

Speakers
PulseSink::getInput(void) const
{
  return spk;
}

bool PulseSink::process(const Chunk *_chunk)
{
  /////////////////////////////////////////////////////////
  // We may not take output lock here because close() tries
  // to take playback lock before closing audio output.

  AutoLock autolock(&playback_lock);

  if ( _chunk->isDummy() )
    return true;

  /////////////////////////////////////////////////////////
  // process() automatically opens audio output if it is
  // not open. It is because input format in uninitialized
  // state = spk_unknown. It is exactly what we want.

  if ( _chunk->spk != spk )
  {
    if ( ! setInput(_chunk->spk) )
      return false;
  }

  void *data1, *data2;
  DWORD data1_bytes, data2_bytes;
  DWORD play_cur;
  DWORD data_size;

  size_t  size = _chunk->size;
  uint8_t *buf = _chunk->rawdata;

  while ( size )
  {
    ///////////////////////////////////////////////////////
    // Here we put chunk data to DirectSound buffer. If
    // it is too much of chunk data we have to put it by
    // parts. When we put part of data we wait until some
    // data is played and part of buffer frees so we can
    // put next part.

    ///////////////////////////////////////////////////////
    // Determine how much data to output (data_size)
    // (check free space in playback buffer and size of
    // remaining input data)

    if FAILED(ds_buf->GetCurrentPosition(&play_cur, 0))
      return false;

    if (play_cur > cur)
      data_size = play_cur - cur;
    else if (play_cur < cur)
      data_size = buf_size + play_cur - cur;
    else
      // if playback cursor is equal to buffer write position it may mean:
      // * playback is stopped/paused so both pointers are equal
	  // (free buffer = buf_size)
      // * buffer is full so both pointers are equal (free buffer = 0)
      // * buffer underrun (we do not take this in account because in either case
      //   it produces playback glitch)
      data_size = playing? 0: buf_size;

    if ( data_size > size )
      data_size = (DWORD)size;

    ///////////////////////////////////////////////////////
    // Put data to playback buffer

    if ( data_size )
    {
      if FAILED(ds_buf->Lock(cur, data_size, &data1, &data1_bytes, &data2, &data2_bytes, 0))
        return false;

      memcpy(data1, buf, data1_bytes);
      buf += data1_bytes;
      size -= data1_bytes;
      cur += data1_bytes;

      if ( data2_bytes )
      {
        memcpy(data2, buf, data2_bytes);
        buf += data2_bytes;
        size -= data2_bytes;
        cur += data2_bytes;
      }

      if FAILED(ds_buf->Unlock(data1, data1_bytes, data2, data2_bytes))
        return false;
    }

    ///////////////////////////////////////////////////////
    // Start playback after prebuffering

    if ( ! playing && cur > preload_size )
    {
      if ( ! paused )
        ds_buf->Play(0, 0, DSBPLAY_LOOPING);
      playing = true;
    }

    if (cur >= buf_size)
      cur -= buf_size;

    ///////////////////////////////////////////////////////
    // Now we have either:
    // * some more data to output & full playback buffer
    // * no more data to output
    //
    // If we have some input data remaining to output we
    // need to wait until some data is played back to continue

    if ( size )
    {
      // Sleep until we can put all remaining data or
      // we have free at least half of playback buffer
      // Note that we must finish immediately on ev_stop

      data_size = (DWORD)min(size, buf_size / 2);
      //if (WaitForSingleObject(ev_stop, DWORD(data_size * bytes2time * 1000 + 1)) == WAIT_OBJECT_0)
        //return true;
      continue;
    }
  }

  if ( _chunk->sync )
    time = _chunk->time + _chunk->size * bytes2time;
  else
    time += _chunk->size * bytes2time;

  if ( _chunk->eos )
    flush();

  return true;
}

