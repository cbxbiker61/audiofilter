/*
  Demuxer - demux MPEG container (filter wrapper for MPEGDemux class)
  MPEG1/2 PES supported 
  todo: MPEG2 transport stream

  Input:     PES
  Output:    AC3, MPA, DTS, PCM16_LE PCM24_LE
  OFDD:      yes
  Buffering: inplace
  Timing:    passthrough
  Parameters:
    [ro] stream - current stream number
    [ro] substream - current substream number
*/

#ifndef VALIB_DEMUX_H
#define VALIB_DEMUX_H

#include "../filter.h"
#include "../mpeg_demux.h"

class Demux : public NullFilter
{
protected:
  PSParser ps;          // MPEG Program Stream parser

  int    stream;        // current stream
  int    substream;     // current substream

  Speakers out_spk;
  uint8_t *out_rawdata;
  size_t   out_size;

  void process();

public:
  Demux();

  inline int get_stream()    const { return stream;    }
  inline int get_substream() const { return substream; }
    
  // Filter interface
  virtual void reset();
  virtual bool is_ofdd() const;
  virtual bool process(const Chunk *chunk);

  virtual Speakers get_output() const;
  virtual bool is_empty() const;
  virtual bool get_chunk(Chunk *chunk);
};

#endif
