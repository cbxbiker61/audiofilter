#pragma once
#ifndef AUDIOFILTER_RAWSINK_H
#define AUDIOFILTER_RAWSINK_H
/*
  RAW file output audio renderer
*/

#include "AutoFile.h"
#include "Filter.h"

namespace AudioFilter {

class RawSink : public Sink
{
public:
  RawSink()
  {}

  RawSink(const char *filename)
    : _f(filename, "wb")
  {}

  RawSink(FILE *f)
    : _f(f)
  {}

  bool open(const char *filename)
  {
    return _f.open(filename, "wb");
  }

  bool open(FILE *f)
  {
    return _f.open(f);
  }

  void close(void)
  {
    _f.close();
    _spk = Speakers::UNKNOWN;
  }

  bool isOpen(void) const
  {
    return _f.isOpen();
  }

  /////////////////////////////////////////////////////////
  // Sink interface

  virtual bool queryInput(Speakers spk) const
  {
    return _f.isOpen() && spk.isRawData(); // cannot write linear format
  }

  virtual bool setInput(Speakers spk)
  {
    if ( queryInput(spk) )
    {
      _spk = spk;
      return true;
    }

    return false;
  }

  virtual Speakers getInput(void) const
  {
    return _spk;
  }

  // data write
  virtual bool process(const Chunk *chunk)
  {
    if ( ! chunk->isDummy() )
    {
      if ( _spk == chunk->spk || setInput(chunk->spk) )
        return _f.write(chunk->rawdata, chunk->size) == chunk->size;

      return false;
    }

    return true;
  }

protected:
  Speakers _spk;
  AutoFile _f;
};

}; // namespace AudioFilter

#endif

// vim: ts=2 sts=2 et

