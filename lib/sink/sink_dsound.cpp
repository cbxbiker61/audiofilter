#include <math.h>
#include "sink_dsound.h"
#include "../win32/winspk.h"

#define SAFE_RELEASE(p) { if (p) p->Release(); p = 0; }


DSoundSink::DSoundSink()
{
  hwnd         = 0;
  device       = 0;
  buf_size_ms  = 0;
  preload_ms   = 0;

  spk          = spk_unknown;
  buf_size     = 0;
  preload_size = 0;
  bytes2time   = 0.0;

  ds      = 0;
  ds_buf  = 0;

  cur     = 0;
  time    = 0;
  playing = false;
  paused  = true;

  ev_stop = CreateEvent(0, true, false, 0);
}

DSoundSink::~DSoundSink()
{
  close();
  close_dsound();

  CloseHandle(ev_stop);
}

///////////////////////////////////////////////////////////////////////////////
// Resource allocation

bool
DSoundSink::open_dsound(HWND _hwnd, int _buf_size_ms, int _preload_ms, LPCGUID _device)
{
  if (ds)
    close_dsound();

  // Open DirectSound

  if FAILED(DirectSoundCreate(_device, &ds, 0))
    return false;

  if (!_hwnd) _hwnd = GetForegroundWindow();
  if (!_hwnd) _hwnd = GetDesktopWindow();
  if FAILED(ds->SetCooperativeLevel(_hwnd, DSSCL_PRIORITY))
  {
    SAFE_RELEASE(ds);
    return false;
  }

  buf_size_ms = _buf_size_ms;
  preload_ms = _preload_ms;
  device = _device;
  return true;
}

void
DSoundSink::close_dsound()
{
  close();
  if (ds)
    SAFE_RELEASE(ds);
}

bool
DSoundSink::open(Speakers _spk)
{
  if (!ds) return false;
  AutoLock autolock(&dsound_lock);

  WAVEFORMATEXTENSIBLE wfx;
  memset(&wfx, 0, sizeof(wfx));

  close();
  spk = _spk;

  if (spk2wfx(_spk, (WAVEFORMATEX*)(&wfx), true))
    if (open((WAVEFORMATEX*)(&wfx)))
      return true;

  if (spk2wfx(_spk, (WAVEFORMATEX*)&wfx, false))
    if (open((WAVEFORMATEX*)(&wfx)))
      return true;

  close();
  return false;
}

bool
DSoundSink::open(WAVEFORMATEX *wf)
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
DSoundSink::try_open(Speakers _spk) const
{
  if (!ds) return false;

  // This function do not use shared DirectSound buffer and
  // therefore we do not take any lock here.

  WAVEFORMATEXTENSIBLE wfx;
  memset(&wfx, 0, sizeof(wfx));

  if (spk2wfx(_spk, (WAVEFORMATEX*)(&wfx), true))
    if (try_open((WAVEFORMATEX*)(&wfx)))
      return true;

  if (spk2wfx(_spk, (WAVEFORMATEX*)&wfx, false))
    if (try_open((WAVEFORMATEX*)(&wfx)))
      return true;

  return false;
}

bool
DSoundSink::try_open(WAVEFORMATEX *wf) const
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
DSoundSink::close()
{
  AutoLock autolock(&dsound_lock);

  /////////////////////////////////////////////////////////
  // Unblock playback and take playback lock. It is safe
  // because ev_stop remains signaled (manual-reset event)
  // and playback functions cannot block anymore.

  SetEvent(ev_stop);
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

  /////////////////////////////////////////////////////////
  // Reset ev_stop

  ResetEvent(ev_stop);
}

///////////////////////////////////////////////////////////////////////////////
// Own interface

///////////////////////////////////////////////////////////////////////////////
// Playback control

void
DSoundSink::pause()
{
  AutoLock autolock(&dsound_lock);
  if (ds_buf)
    ds_buf->Stop();
  paused = true;
}

void 
DSoundSink::unpause()
{
  AutoLock autolock(&dsound_lock);
  if (ds_buf && playing)
    ds_buf->Play(0, 0, DSBPLAY_LOOPING);
  paused = false;
}

bool 
DSoundSink::is_paused() const
{
  return paused;
}

vtime_t 
DSoundSink::get_playback_time() const
{
  return time - get_data_time();
}

size_t 
DSoundSink::get_buffer_size() const
{
  return buf_size;
}

vtime_t
DSoundSink::get_buffer_time() const
{
  return buf_size * bytes2time;
}


size_t 
DSoundSink::get_data_size() const
{
  if (!ds_buf) return 0;
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
DSoundSink::get_data_time() const
{
  return get_data_size() * bytes2time;
}

void 
DSoundSink::stop()
{
  AutoLock autolock(&dsound_lock);

  /////////////////////////////////////////////////////////
  // Unblock playback and take playback lock. It is safe
  // because ev_stop remains signaled (manual-reset event)
  // and playback functions cannot block anymore.

  SetEvent(ev_stop);
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

  ResetEvent(ev_stop);
}

void 
DSoundSink::flush()
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

  if (data_size > buf_size / 2)
  {
    data_size -= buf_size / 2;
    if (WaitForSingleObject(ev_stop, DWORD(data_size * bytes2time * 1000 + 1)) == WAIT_OBJECT_0)
      return;
  }

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

  if (!playing)
  {
    ds_buf->Play(0, 0, DSBPLAY_LOOPING);
    playing = true;
  }

  /////////////////////////////////////////////////////////
  // Sleep until the end of playback
  // Note that we must finish immediately on ev_stop

  data_size = buf_size - data_size;
  if (WaitForSingleObject(ev_stop, DWORD(data_size * bytes2time * 1000 + 1)) == WAIT_OBJECT_0)
    return;

  /////////////////////////////////////////////////////////
  // Stop the playback

  stop();
}

double 
DSoundSink::get_vol() const
{
  if (!ds_buf) return 0;
  AutoLock autolock(&dsound_lock);

  LONG vol;
  if SUCCEEDED(ds_buf->GetVolume(&vol))
    return (double)(vol / 100);
  return 0;
}

void 
DSoundSink::set_vol(double vol)
{
  if (!ds_buf) return;
  AutoLock autolock(&dsound_lock);
  ds_buf->SetVolume((LONG)(vol * 100));
}

double 
DSoundSink::get_pan() const
{
  if (!ds_buf) return 0;
  AutoLock autolock(&dsound_lock);

  LONG pan;
  if SUCCEEDED(ds_buf->GetPan(&pan))
    return (double)(pan / 100);
  return 0;
}

void 
DSoundSink::set_pan(double pan)
{
  if (!ds_buf) return;
  AutoLock autolock(&dsound_lock);
  ds_buf->SetPan((LONG)(pan * 100));
}


///////////////////////////////////////////////////////////////////////////////
// Sink interface

bool 
DSoundSink::query_input(Speakers _spk) const
{
  return try_open(_spk);
}

bool 
DSoundSink::set_input(Speakers _spk)
{
  AutoLock autolock(&dsound_lock);

  close();
  return open(_spk);
}

Speakers
DSoundSink::get_input() const
{
  return spk;
}

bool DSoundSink::process(const Chunk *_chunk)
{
  /////////////////////////////////////////////////////////
  // We may not take output lock here because close() tries
  // to take playback lock before closing audio output.

  AutoLock autolock(&playback_lock);

  if (_chunk->is_dummy())
    return true;

  /////////////////////////////////////////////////////////
  // process() automatically opens audio output if it is
  // not open. It is because input format in uninitialized
  // state = spk_unknown. It is exactly what we want.

  if (_chunk->spk != spk)
    if (!set_input(_chunk->spk))
      return false;

  void *data1, *data2;
  DWORD data1_bytes, data2_bytes;
  DWORD play_cur;
  DWORD data_size;

  size_t  size = _chunk->size;
  uint8_t *buf = _chunk->rawdata;

  while (size)
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
      // * playback is stopped/paused so both pointers are equal (free buffer = buf_size)
      // * buffer is full so both pointers are equal (free buffer = 0)
      // * buffer underrun (we do not take this in account because in either case
      //   it produces playback glitch)
      data_size = playing? 0: buf_size;

    if (data_size > size)
      data_size = (DWORD)size;

    ///////////////////////////////////////////////////////
    // Put data to playback buffer

    if (data_size)
    {
      if FAILED(ds_buf->Lock(cur, data_size, &data1, &data1_bytes, &data2, &data2_bytes, 0))
        return false;

      memcpy(data1, buf, data1_bytes);
      buf += data1_bytes;
      size -= data1_bytes;
      cur += data1_bytes;

      if (data2_bytes)
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

    if (!playing && cur > preload_size)
    {
      if (!paused)
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

    if (size)
    {
      // Sleep until we can put all remaining data or 
      // we have free at least half of playback buffer
      // Note that we must finish immediately on ev_stop

      data_size = (DWORD)min(size, buf_size / 2);
      if (WaitForSingleObject(ev_stop, DWORD(data_size * bytes2time * 1000 + 1)) == WAIT_OBJECT_0)
        return true;
      continue;
    }
  }

  if (_chunk->sync)
    time = _chunk->time + _chunk->size * bytes2time;
  else
    time += _chunk->size * bytes2time;

  if (_chunk->eos)
    flush();

  return true;
}

