#pragma once
#ifndef AUDIOFILTER_SPDIFFRAMEPARSER_H
#define AUDIOFILTER_SPDIFFRAMEPARSER_H
/*
  SPDIF parser class
  Converts SPDIF stream to raw AC3/MPA/DTS stream.
*/

#include "Parsers.h"

namespace AudioFilter {

class SpdifFrameParser : public FrameParser
{
public:

  SpdifFrameParser( bool isBigEndian );
  ~SpdifFrameParser();

  bool isBigEndian(void) const
  {
    return _isBigEndian;
  }

  void setBigEndian(bool isBe)
  {
    _isBigEndian = isBe;
  }

  HeaderInfo getHeaderInfo(void) const
  {
    return _hi;
  }

  virtual HeaderParser *getHeaderParser(void);
  virtual void reset(void);
  virtual bool parseFrame(uint8_t *frame, size_t size);

  virtual Speakers getSpk(void) const
  {
    return _hi.spk;
  }

  virtual samples_t getSamples(void) const
  {
    samples_t samples;
    samples.zero();
    return samples;
  }

  virtual size_t getNSamples(void) const
  {
    return _hi.nsamples;
  }

  virtual uint8_t *getRawData(void) const
  {
    return _data.ptr;
  }

  virtual size_t getRawSize(void) const
  {
    return _data.size;
  }

  virtual std::string getStreamInfo(void) const;
  virtual std::string getFrameInfo(void) const;

protected:
  struct Data
  {
    Data() : ptr(0), size(0) {}
    uint8_t *ptr;
    size_t size;
  } _data;

  HeaderInfo _hi;

private:
  SpdifHeaderParser _spdifHeaderParser;
  bool _isBigEndian;
};

}; // namespace AudioFilter

#endif

// vim: ts=2 sts=2 et

