#pragma once
#ifndef AUDIOFILTER_WAVSINK_H
#define AUDIOFILTER_WAVSINK_H

#include "Filter.h"
#include "AutoFile.h"

namespace AudioFilter {

class WavSink : public Sink
{
public:
  WavSink();
  WavSink(const char *filename);
  ~WavSink();

  bool open(const char *filename);
  void close(void);
  bool isOpen(void) const;

  /////////////////////////////////////////////////////////
  // Sink interface

  virtual bool queryInput(Speakers _spk) const;
  virtual bool setInput(Speakers _spk);
  virtual Speakers getInput(void) const;
  virtual bool process(const Chunk *_chunk);

protected:
  void initRiff(void);
  void closeRiff(void);

  AutoFile f;
  Speakers spk;

  uint32_t header_size; // WAV header size;
  uint64_t data_size;  // data size written to the file
  uint8_t *file_format; // WAVEFORMAT *

};

}; // namespace AudioFilter

#endif

// vim: ts=2 sts=2 et

