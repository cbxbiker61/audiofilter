/*
  AudioDecoder
  Universal audio decoder
*/

#ifndef VALIB_DECODER_H
#define VALIB_DECODER_H

#include "parser_filter.h"
#include "../parsers/mpa/mpa_parser.h"
#include "../parsers/ac3/ac3_parser.h"
#include "../parsers/dts/dts_parser.h"
#include "../parsers/multi_frame.h"


class AudioDecoder : public Filter
{
protected:
  ParserFilter parser;

  MPAParser mpa;
  AC3Parser ac3;
  DTSParser dts;
  MultiFrame multi_parser;

public:
  AudioDecoder()
  {
    FrameParser *parsers[] = { &ac3, &dts, &mpa };
    multi_parser.set_parsers(parsers, array_size(parsers));
    parser.set_parser(&multi_parser);
  }

  int        get_frames()                    const { return parser.get_frames();       }
  int        get_errors()                    const { return parser.get_errors();       }

  size_t     get_info(char *buf, size_t len) const { return parser.get_info(buf, len); }
  HeaderInfo header_info()                   const { return parser.header_info();      }

  /////////////////////////////////////////////////////////
  // Filter interface

  virtual void reset()                     { parser.reset();                 }
  virtual bool is_ofdd() const             { return parser.is_ofdd();        }

  virtual bool query_input(Speakers spk) const { return parser.query_input(spk); }
  virtual bool set_input(Speakers spk)     { return parser.set_input(spk);   }
  virtual Speakers get_input() const       { return parser.get_input();      }
  virtual bool process(const Chunk *chunk) { return parser.process(chunk);   }

  virtual Speakers get_output() const      { return parser.get_output();     }
  virtual bool is_empty() const            { return parser.is_empty();       }
  virtual bool get_chunk(Chunk *chunk)     { return parser.get_chunk(chunk); }
};

#endif
