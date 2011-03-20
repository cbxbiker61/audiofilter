#pragma once
#ifndef AUDIOFILTER_MPAFRAMEPARSER_H
#define AUDIOFILTER_MPAFRAMEPARSER_H

/*
 * MPEG Audio parser
 * MPEG1/2 LayerI and LayerII audio parser class
 */

#include <string>
#include "BitStream.h"
#include "Buffer.h"
#include "MpaDefs.h"
#include "MpaSynth.h"
#include "Parsers.h"

namespace AudioFilter {

class MpaFrameParser : public FrameParser
{
public:
  MpaFrameParser();
  ~MpaFrameParser();

  // raw frame header struct
  union Header
  {
    Header() {}
    Header(uint32_t i)
    {
      raw = i;
    }

    uint32_t raw;
    struct
    {
      unsigned emphasis           : 2;
      unsigned original           : 1;
      unsigned copyright          : 1;
      unsigned mode_ext           : 2;
      unsigned mode               : 2;
      unsigned extension          : 1;
      unsigned padding            : 1;
      unsigned sampling_frequency : 2;
      unsigned bitrate_index      : 4;
      unsigned error_protection   : 1;
      unsigned layer              : 2;
      unsigned version            : 1;
      unsigned sync               : 12;
    };
  };

  // bitstream information struct
  struct BSI
  {
    int bs_type;    // bitstream type
    int ver;        // stream version: 0 = MPEG1; 1 = MPEG2 LSF
    int layer;      // indicate layer (MPA_LAYER_X constants)
    int mode;       // channel mode (MPA_MODE_X constants)
    int bitrate;    // bitrate [bps]
    int freq;       // sampling frequency [Hz]

    int nch;        // number of channels
    int nsamples;   // number of samples in sample buffer (384 for Layer1, 1152 for LayerII)
    size_t frame_size; // frame size in bytes (not slots!)

    int jsbound;    // first band of joint stereo coding
    int sblimit;    // total number of subbands
  };

private:
  Header _hdr; // raw header
  BSI _bsi; // bitstream information

public:
  /////////////////////////////////////////////////////////
  // FrameParser overrides

  virtual const HeaderParser *getHeaderParser(void) const;

  virtual void reset(void);
  virtual bool parseFrame(uint8_t *frame, size_t size);

  virtual Speakers  getSpk(void) const
  {
    return _spk;
  }

  virtual samples_t getSamples(void) const
  {
    return _samples;
  }

  virtual size_t getNSamples(void) const
  {
    return _bsi.nsamples;
  }

  virtual uint8_t *getRawData(void) const
  {
    return 0;
  }

  virtual size_t getRawSize(void) const
  {
    return 0;
  }

  virtual std::string getStreamInfo(void) const;
  virtual std::string getFrameInfo(void) const;

private:

  bool parseHeader(const uint8_t *frame, size_t size);
  bool checkCrc(const uint8_t *frame, size_t protected_data_bits) const;

  bool decodeFrame_I(const uint8_t *frame);
  void decodeFraction_I(
         sample_t *fraction[MPA_NCH], // pointers to sample_t[SBLIMIT] arrays
         int16_t  bit_alloc[MPA_NCH][SBLIMIT],
         sample_t scale[MPA_NCH][SBLIMIT]);

  bool decodeFrame_II(const uint8_t *frame);
  void decodeFraction_II(
         sample_t *fraction[MPA_NCH], // pointers to sample_t[SBLIMIT*3] arrays
         int16_t  bit_alloc[MPA_NCH][SBLIMIT],
         sample_t scale[MPA_NCH][3][SBLIMIT],
         int x);

  MpaHeaderParser _mpaHeaderParser;
  Speakers _spk; // output format
  SampleBuf _samples; // samples buffer
  ReadBS _bs; // bitstream reader

  SynthBuffer *_synth[MPA_NCH]; // synthesis buffers
  int _II_table; // Layer II allocation table number
};

}; // namespace AudioFilter

#endif

// vim: ts=2 sts=2 et

