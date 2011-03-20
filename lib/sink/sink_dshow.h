/*
  DirectShow output sink
*/

#ifndef VALIB_SINK_DSHOW_H
#define VALIB_SINK_DSHOW_H

#include <windows.h>
#include <streams.h>
#include "../filter.h"

bool mt2spk(CMediaType mt, Speakers &spk);
bool spk2mt(Speakers spk, CMediaType &mt, bool use_wfx);

class DShowSink : public CTransformOutputPin, public Sink
{
protected:
  Speakers spk;             // output configuration
  bool send_mt;             // send media type with next sample
  bool send_dc;             // send discontinuity with next sample
  HRESULT hr;               // result of sending sample

  bool query_downstream(const CMediaType *mt) const;
  bool set_downstream(const CMediaType *mt);

  HRESULT CheckMediaType(const CMediaType *mt);
  HRESULT SetMediaType(const CMediaType *mt);

public:
  DShowSink(CTransformFilter* pFilter, HRESULT* phr);

  void send_discontinuity()          { send_dc = true; }
  void send_mediatype()              { send_mt = true; }

  void reset_hresult()               { hr = S_OK;      }
  HRESULT get_hresult()              { return hr;      }

  // Sink interface
  virtual bool query_input(Speakers spk) const;
  virtual bool set_input(Speakers spk);
  virtual Speakers get_input() const;
  virtual bool process(const Chunk *chunk);
};

#endif
