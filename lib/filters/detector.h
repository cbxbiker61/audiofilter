/*
  Detector

  Format autodetection. Does not alter data, but detects compressed stream and
  its format. Main purpose is to detect SPDIF stream at PCM16 data.
*/

#ifndef VALIB_DETECTOR_H
#define VALIB_DETECTOR_H

#include "../parser.h"
#include "../sync.h"
#include "../parsers/multi_header.h"



class Detector : public NullFilter
{
protected:
  MultiHeader spdif_dts_header;
  MultiHeader uni_header;

  enum state_t 
  {
    state_trans,
    state_empty_debris, state_debris, state_no_debris,
    state_empty_frame, state_frame_debris, state_frame, state_no_frame,
    state_format_change
  };

  StreamBuffer stream;    // stream buffer

  Speakers out_spk;       // output format
  state_t  state;         // filter state
  Sync     sync_helper;   // syncronization helper

  const HeaderParser *find_parser(Speakers spk) const;

  bool load();
  void send_debris(Chunk *_chunk);
  void send_frame_debris(Chunk *_chunk);
  void send_frame(Chunk *_chunk);
  void send_eos(Chunk *_chunk);

public:
  Detector();
  ~Detector();

  /////////////////////////////////////////////////////////
  // Own interface

  int  get_frames() const { return stream.get_frames(); }
  size_t get_info(char *buf, size_t len) const { return stream.stream_info(buf, len); }
  HeaderInfo header_info() const { return stream.header_info(); }

  /////////////////////////////////////////////////////////
  // Filter interface

  virtual void reset();
  virtual bool is_ofdd() const;

  virtual bool query_input(Speakers spk) const;
  virtual bool set_input(Speakers spk);
  virtual bool process(const Chunk *chunk);

  virtual Speakers get_output() const;
  virtual bool is_empty() const;
  virtual bool get_chunk(Chunk *chunk);
};

#endif
