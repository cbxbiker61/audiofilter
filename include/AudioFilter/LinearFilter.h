#pragma once
#ifndef AUDIOFILTER_LINEARFILTER_H
#define AUDIOFILTER_LINEARFILTER_H

#include "Filter.h"
#include "SyncHelper.h"

namespace AudioFilter {

///////////////////////////////////////////////////////////////////////////////
// Simplified filter interface for linear processing

class LinearFilter : public Filter
{
public:
  LinearFilter();
  virtual ~LinearFilter();

  /////////////////////////////////////////////////////////////////////////////
  // Filter interface (fully implemented)

  virtual void reset(void);
  virtual bool isOfdd(void) const;

  virtual bool queryInput(Speakers spk) const;
  virtual bool setInput(Speakers spk);
  virtual Speakers getInput(void) const;
  virtual bool process(const Chunk *chunk);

  virtual Speakers getOutput(void) const;
  virtual bool isEmpty(void) const;
  virtual bool getChunk(Chunk *chunk);

protected:
  Speakers getInSpk(void) const
  {
    return in_spk;
  }

  Speakers getOutSpk(void) const
  {
    return out_spk;
  }

  bool reinit(bool format_change);

  /////////////////////////////////////////////////////////////////////////////
  // Interface to override

  virtual bool query(Speakers spk) const;
  virtual bool init(Speakers spk, Speakers &out_spk);
  virtual void resetState(void);

  virtual void sync(vtime_t time);
  virtual bool processSamples(samples_t in, size_t in_size, samples_t &out, size_t &out_size, size_t &gone);
  virtual bool processInplace(samples_t in, size_t in_size);
  virtual bool flush(samples_t &out, size_t &out_size);

  virtual bool needFlushing(void) const;

private:
  SyncHelper sync_helper;

  Speakers   in_spk;
  Speakers   out_spk;
  samples_t  samples;
  size_t     size;
  samples_t  out_samples;
  size_t     out_size;
  size_t     buffered_samples;
  int flushing;

  bool process(void);
  bool flush(void);
};

}; // namespace AudioFilter

#endif

// vim: ts=2 sts=2 et

