#pragma once
#ifndef AUDIOFILTER_SPDIFER_H
#define AUDIOFILTER_SPDIFER_H
/*
  Wrap/unwrap encoded stream to/from SPDIF according to IEC 61937

  Input:     SPDIF, AC3, MPA, DTS, Unknown
  Output:    SPDIF, DTS
  OFDD:      yes
  Buffering: block
  Timing:    first syncpoint
  Parameters:
    -
*/

#include "parser_filter.h"
#include "../parsers/spdif/spdif_wrapper.h"
#include "../parsers/spdif/spdif_parser.h"

namespace AudioFilter {

class Spdifer : public Filter
{
public:
  Spdifer()
  {
    parser.setParser(&spdif_wrapper);
  }

  /////////////////////////////////////////////////////////
  // Spdifer interface

  int getDtsMode(void) const
  {
    return spdif_wrapper.dts_mode;
  }

  void setDtsMode(int dts_mode)
  {
    spdif_wrapper.dts_mode = dts_mode;
  }

  int getDtsConv(void) const
  {
    return spdif_wrapper.dts_conv;
  }

  void setDtsConv(int dts_conv)
  {
    spdif_wrapper.dts_conv = dts_conv;
  }

  int getFrames(void) const
  {
    return parser.get_frames();
  }

  int getErrors(void) const
  {
    return parser.get_errors();
  }

  size_t getInfo(char *buf, size_t len) const
  {
    return parser.get_info(buf, len);
  }

  HeaderInfo getHeaderInfo(void) const
  {
    return parser.header_info();
  }

  /////////////////////////////////////////////////////////
  // Filter interface

  virtual void reset(void)                  { parser.reset();                 }
  virtual bool isOfdd(void) const           { return parser.isOfdd();        }

  virtual bool queryInput(Speakers spk) const { return parser.queryInput(spk); }
  virtual bool setInput(Speakers spk)     { return parser.setInput(spk);   }
  virtual Speakers getInput(void) const       { return parser.getInput();      }
  virtual bool process(const Chunk *chunk) { return parser.process(chunk);   }

  virtual Speakers getOutput(void) const      { return parser.getOutput();     }
  virtual bool isEmpty(void) const            { return parser.isEmpty();       }
  virtual bool getChunk(Chunk *chunk)     { return parser.getChunk(chunk); }

protected:
  ParserFilter parser;
  SPDIFWrapper spdif_wrapper;

};

class DeSpdifer : public Filter
{
public:
  DeSpdifer(): spdif_parser(true)
  {
    parser.setParser(&spdif_parser);
  }

  bool getBigEndian(void) const
  {
    return spdif_parser.getBigEndian();
  }

  void setBigEndian(bool _big_endian)
  {
    spdif_parser.setBigEndian(_big_endian);
  }

  int getFrames(void) const
  {
    return parser.getFrames();
  }

  int getErrors(void) const
  {
    return parser.getErrors();
  }

  size_t getInfo(char *buf, size_t len) const
  {
    return parser.getInfo(buf, len);
  }

  HeaderInfo getHeaderInfo(void) const
  {
    return parser.getHeaderInfo();
  }

  /////////////////////////////////////////////////////////
  // Filter interface

  virtual void reset(void)
  {
    parser.reset();
  }

  virtual bool isOfdd(void) const
  {
    return parser.isOfdd();
  }

  virtual bool queryInput(Speakers spk) const
  {
    return parser.queryInput(spk);
  }

  virtual bool setInput(Speakers spk)
  {
    return parser.setInput(spk);
  }

  virtual Speakers getInput(void) const
  {
    return parser.getInput();
  }

  virtual bool process(const Chunk *chunk)
  {
    return parser.process(chunk);
  }

  virtual Speakers getOutput(void) const
  {
    return parser.getOutput();
  }

  virtual bool isEmpty(void) const
  {
    return parser.isEmpty();
  }

  virtual bool getChunk(Chunk *chunk)
  {
    return parser.getChunk(chunk);
  }

protected:
  ParserFilter parser;
  SPDIFParser spdif_parser;

};

}; // namespace AudioFilter

#endif

// vim: ts=2 sts=2 et

