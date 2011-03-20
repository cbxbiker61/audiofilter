#pragma once
#ifndef AUDIOFILTER_WAVSOURCE_H
#define AUDIOFILTER_WAVSOURCE_H

/*
 * WAV file source
 */

#include <AudioFilter/AutoFile.h>
#include <AudioFilter/Buffer.h>
#include <AudioFilter/Filter.h>

namespace AudioFilter {

class WavSource : public Source
{
public:
  WavSource();
  WavSource(const char *filename, size_t block_size);

  bool open(const char *filename, size_t block_size);
  void close(void);
  bool isOpen(void) const;

  AutoFile::fsize_t size(void) const;
  AutoFile::fsize_t pos(void) const;
  int seek(AutoFile::fsize_t pos);

  /////////////////////////////////////////////////////////
  // Source interface

  Speakers getOutput(void) const;
  bool isEmpty(void) const;
  bool getChunk(Chunk *chunk);

protected:
  bool openRiff(void);

  AutoFile f;
  UInt8Buf buf;
  Speakers spk;

  size_t block_size;

  AutoFile::fsize_t data_start;
  uint64_t data_size;
  uint64_t data_remains;

};

}; // namespace AudioFilter

#endif

// vim: ts=2 sts=2 et

