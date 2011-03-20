/*
   DecoderGraph
   Simple decoding graph that accepts PCM/SPDIF/encoded formats at input and
   allows audio processing. May be used instead of DVDGraph when we don't need
   SPDIF output and output format agreement.
*/

#ifndef VALIB_DECODER_GRAPH_H
#define VALIB_DECODER_GRAPH_H

#include "../filter_graph.h"
#include "decoder.h"
#include "spdifer.h"
#include "proc.h"

class DecoderGraph : public FilterGraph
{
public:
  Despdifer      despdifer;
  AudioDecoder   dec;
  AudioProcessor proc;

public:
  DecoderGraph();

  /////////////////////////////////////////////////////////////////////////////
  // DecoderGraph interface

  // User format
  bool query_user(Speakers user_spk) const;
  bool set_user(Speakers user_spk);
  Speakers get_user() const;

  // Summary information
  size_t get_info(char *_buf, size_t _len) const;

  /////////////////////////////////////////////////////////////////////////////
  // Filter overrides

  virtual void reset();

protected:
  Speakers user_spk;

  enum state_t 
  { 
    state_despdif,
    state_decode,
    state_proc,
  };            

  /////////////////////////////////////////////////////////////////////////////
  // FilterGraph overrides

  virtual const char *get_name(int node) const;
  virtual Filter *init_filter(int node, Speakers spk);
  virtual int get_next(int node, Speakers spk) const;
};

#endif
