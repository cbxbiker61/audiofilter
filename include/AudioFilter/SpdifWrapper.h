#pragma once
#ifndef AUDIOFILTER_SPDIFWRAPPER_H
#define AUDIOFILTER_SPDIFWRAPPER_H
/*
 * SPDIF wrapper class
 * Converts raw AC3/MPA/DTS stream to SPDIF stream.
 * Does raw stream output if the stream given cannot be spdifed
 * (high-bitrate DTS for example). Can convert DTS stream type
 * to 14/16 bits. Supports padded and wrapped SPDIF stream types.
 * Can re-wrap SPDIF stream with conversion between SPDIF stream types.
 */

#include <vector>
#include "Parsers.h"
#include "SpdifFrameParser.h"

namespace AudioFilter {

#define DTS_MODE_AUTO    0
#define DTS_MODE_WRAPPED 1
#define DTS_MODE_PADDED  2

#define DTS_CONV_NONE    0
#define DTS_CONV_16BIT   1
#define DTS_CONV_14BIT   2

class SpdifWrapper : public FrameParser
{
public:

  SpdifWrapper(HeaderParser *hp = 0
                  , int dtsMode = DTS_MODE_AUTO
                  , int dtsConv = DTS_CONV_NONE);
  ~SpdifWrapper();

  void addChannelMap( int channels, int sampleRate )
  {
      _channelMap.push_back(ChannelMap(channels, sampleRate));
  }

  HeaderInfo getHeaderInfo(void) const
  {
    return _hi;
  }

  virtual HeaderParser *getHeaderParser(void);

  virtual void reset(void);
  virtual bool parseFrame(uint8_t *frame, size_t size);

  virtual const Speakers &getSpeakers(void) const
  {
    return _spk;
  }

  virtual samples_t getSamples(void) const
  {
    samples_t samples;
    samples.zero();
    return samples;
  }

  virtual size_t getSampleCount(void) const
  {
    return _hi.getSampleCount();
  }

  virtual uint8_t *getRawData(void) const
  {
    return _spdifFrame.ptr;
  }

  virtual size_t getRawSize(void) const
  {
    return _spdifFrame.size;
  }

  virtual std::string getStreamInfo(void) const;
  virtual std::string getFrameInfo(void) const;

private:

  class ChannelMap
  {
  public:
    ChannelMap(int channels, int sampleRate)
      : _channels(channels), _sampleRate(sampleRate)
    {}
  private:
    int _channels;
    int _sampleRate;
  friend class SpdifWrapper;
  };

  int _dtsMode;
  int _dtsConv;

  uint8_t *_buf; // output frame buffer

  HeaderParser *_pHeaderParser; // spdifable formats header parser
  bool _isDefaultHeaderParser; // indicates we're using default header parsers
  //MultiHeader spdifable; // spdifable formats header parser
  SpdifFrameParser _spdifFrameParser; // required to rewrap spdif input
  HeaderInfo _hi; // input raw frame info

  Speakers _spk; // output format

  struct SpdifFrame
  {
    SpdifFrame() : ptr(0), size(0) {}
    uint8_t *ptr;
    size_t size;
  } _spdifFrame;

  bool _useHeader; // use SPDIF header
  int _spdifBsType; // SPDIF bitstream type
private:
  bool isValidChannelMap( int channels, int sampleRate );
  std::vector<ChannelMap> _channelMap;
  size_t _clearOffsetLast;
};

}; // namespace AudioFilter

#endif

// vim: ts=2 sts=2 et

