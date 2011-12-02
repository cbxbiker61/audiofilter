#pragma once
#ifndef AUDIOFILTER_FILEPARSER_H
#define AUDIOFILTER_FILEPARSER_H

/*
  File parser class
*/

#include <cstdio>
#include <string>
#include "AutoFile.h"
#include "Filter.h"
#include "MpegDemux.h"
#include "Parsers.h"

namespace AudioFilter {

class FileParser // : public Source
{
public:
  typedef AutoFile::fsize_t fsize_t;
  size_t max_scan;
  enum units_t { bytes, relative, frames, time };
  inline double getUnitsFactor(units_t units) const;

  FileParser();
  FileParser(const char *filename
            , HeaderParser *parser = 0
            , size_t max_scan = 0);
  ~FileParser();

  bool probe(void);
  bool stats(unsigned max_measurements = 100, vtime_t precision = 0.5);

  bool isOpen(void) const
  {
    return f;
  }

  bool eof(void) const
  {
    return f.eof() && ( buf_pos >= buf_data ) && ! stream.isFrameLoaded();
  }

  const char *getFilename(void) const
  {
    return filename.c_str();
  }

  HeaderParser *getParser(void)
  {
    return stream.getParser();
  }

  std::string getFileInfo(void) const;

  /////////////////////////////////////////////////////////////////////////////
  // Positioning

  fsize_t getPos(void) const;
  double getPos(units_t units) const;

  fsize_t getSize(void) const;
  double getSize(units_t units) const;

  int seek(fsize_t pos);
  int seek(double pos, units_t units);

  /////////////////////////////////////////////////////////////////////////////
  // Frame-level interface (StreamBuffer interface wrapper)

  void reset(void);
  bool loadFrame(void);

  bool isInSync(void) const
  {
    return stream.isInSync();
  }

  bool isNewStream(void) const
  {
    return stream.isNewStream();
  }

  bool isFrameLoaded(void) const
  {
    return stream.isFrameLoaded();
  }

  const Speakers &getSpeakers(void) const
  {
    return stream.getSpeakers();
  }

  uint8_t *getFrame(void) const
  {
    return stream.getFrame();
  }

  size_t getFrameSize(void) const
  {
    return stream.getFrameSize();
  }

  size_t getFrameInterval(void) const
  {
    return stream.getFrameInterval();
  }

  int getFrameCount(void) const
  {
    return stream.getFrameCount();
  }

  std::string getStreamInfo(void) const
  {
    return stream.getStreamInfo();
  }

  HeaderInfo getHeaderInfo(void)
  {
    return stream.getHeaderInfo();
  }

private:
  void init(void);
  bool open(const char *filename
            , HeaderParser *parser
            , size_t max_scan);
  void close(void);

  StreamBuffer stream;

  AutoFile f;
  std::string filename;

  uint8_t *buf;
  size_t buf_size;
  size_t buf_data;
  size_t buf_pos;

  size_t stat_size; // number of measurements done by stat() call
  float avg_frame_interval; // average frame interval
  float avg_bitrate; // average bitrate

  HeaderParser *_intHeaderParser;
};

}; // namespace AudioFilter

#endif

// vim: ts=2 sts=2 et

