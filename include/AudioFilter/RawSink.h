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

  RawSink(const char *_filename)
    : f(_filename, "wb")
  {}

  RawSink(FILE *_f)
    : f(_f)
  {}

  bool open(const char *_filename)
  {
    return f.open(_filename, "wb");
  }

  bool open(FILE *_f)
  {
    return f.open(_f);
  }

  void close(void)
  {
    f.close();
    spk = spk_unknown;
  }

  bool isOpen(void) const
  {
    return f.isOpen();
  }

  /////////////////////////////////////////////////////////
  // Sink interface

  virtual bool queryInput(Speakers _spk) const
  {
    // cannot write linear format
    return f.isOpen() && _spk.format != FORMAT_LINEAR;
  }

  virtual bool setInput(Speakers _spk)
  {
    if ( queryInput(_spk) )
    {
      spk = _spk;
      return true;
    }

    return false;
  }

  virtual Speakers getInput(void) const
  {
    return spk;
  }

  // data write
  virtual bool process(const Chunk *_chunk)
  {
    if ( ! _chunk->isDummy() )
    {
      if ( spk == _chunk->spk || setInput(_chunk->spk) )
        return f.write(_chunk->rawdata, _chunk->size) == _chunk->size;

      return false;
    }

    return true;
  }

protected:
  Speakers spk;
  AutoFile f;

};

}; // namespace AudioFilter

#endif

// vim: ts=2 sts=2 et

