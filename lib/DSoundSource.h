/*
  DirectSound capture source
*/

#ifndef VALIB_DSOUND_SOURCE_H
#define VALIB_DSOUND_SOURCE_H

#include <dsound.h>
#include "../buffer.h"
#include "../filter.h"
#include "../vtime.h"

class DSoundSource : public Source
{
protected:
  /////////////////////////////////////////////////////////
  // Parameters set by user and
  // directly determined from thoose

  Speakers  spk;
  DWORD     buf_size;
  size_t    buf_size_ms;
  double    bytes2time;

  /////////////////////////////////////////////////////////
  // DirectSound

  LPDIRECTSOUNDCAPTURE       ds_capture;
  LPDIRECTSOUNDCAPTUREBUFFER ds_buf;

  /////////////////////////////////////////////////////////
  // Processing variables

  Rawdata out_buf;
  bool    capturing;
  vtime_t time;
  DWORD   cur;

  void zero_all();
  bool open(WAVEFORMATEX *wf, size_t buf_size_ms, LPCGUID device = 0);

public:
  size_t chunk_size_ms;

  DSoundSource();
  DSoundSource(Speakers spk, size_t buf_size_ms, LPCGUID device = 0);
  ~DSoundSource();

  bool is_open() const;
  bool open(Speakers spk, size_t buf_size_ms, LPCGUID device = 0);
  void close();

  bool is_started() const;
  bool start();
  void stop();

  size_t  captured_size() const;
  vtime_t captured_time() const;

  /////////////////////////////////////////////////////////
  // Source interface

  virtual Speakers get_output() const;
  virtual bool is_empty() const;
  virtual bool get_chunk(Chunk *chunk);
};

#endif
