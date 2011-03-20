#pragma once
#ifndef AUDIOFILTER_CONVERT_H
#define AUDIOFILTER_CONVERT_H
/*
  PCM format conversions

  Input formats:   PCMxx, Linear
  Ouptupt formats: PCMxx, Linear
  Format conversions:
    Linear -> PCMxx
    PCMxx  -> Linear
  Default format conversion:
    Linear -> PCM16
  Timing: Preserve original
  Buffering: yes/no
*/

#include <AudioFilter/Buffer.h>
#include <AudioFilter/Filter.h>
#include "ConvertFunc.h"

namespace AudioFilter {

///////////////////////////////////////////////////////////////////////////////
// Converter class
///////////////////////////////////////////////////////////////////////////////

class Converter : public NullFilter
{
protected:
  // conversion function pointer
  void (*convert)(uint8_t *, samples_t, size_t);

  // format
  int  format;             // format to convert to
                           // affected only by set_format() function
  int  order[NCHANNELS];   // channel order to convert to when converting from linear format
                           // channel order to convert from when converting to linear format

  // converted samples buffer
  UInt8Buf buf; // buffer for converted data
  size_t nsamples; // buffer size in samples

  // output data pointers
  uint8_t  *out_rawdata;   // buffer pointer for pcm data
  samples_t out_samples;   // buffer pointers for linear data
  size_t    out_size;      // buffer size in bytes/samples for pcm/linear data

  // part of sample from previous call
  uint8_t   part_buf[48];  // partial sample left from previous call
  size_t    part_size;     // partial sample size in bytes

  convert_t findConversion(int _format, Speakers _spk) const;
  bool initialize(void);       // initialize convertor
  void convertPcm2linear(void);
  void convertLinear2pcm(void);

  bool isLpcm(int format)
  {
    return format == FORMAT_LPCM20 || format == FORMAT_LPCM24;
  }

public:
  Converter(size_t _nsamples);

  /////////////////////////////////////////////////////////
  // Converter interface

  // buffer size
  size_t getBuffer(void) const;
  bool   setBuffer(size_t _nsamples);
  // output format
  int  getFormat(void) const;
  bool setFormat(int _format);
  // output channel order
  void getOrder(int _order[NCHANNELS]) const;
  void setOrder(const int _order[NCHANNELS]);

  /////////////////////////////////////////////////////////
  // Filter interface

  virtual void reset(void);

  virtual bool queryInput(Speakers spk) const;
  virtual bool setInput(Speakers spk);
  virtual bool process(const Chunk *chunk);

  virtual Speakers getOutput(void) const;
  virtual bool getChunk(Chunk *out);
};

}; // namespace AudioFilter

#endif

