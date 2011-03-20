#include <stdio.h>
#include "parser_filter.h"

/*
This filter is data-dependent. Therefore it must follow format change rules
carefully.

Initial state when output format is not known is called transition state.
Also we may switch to transition state when we loose syncrinization and after
flushing.

Filter must flush output on stream change. To correctly finish the stream
format_change state is used.



States list
===========
trans
empty
frame
no_frame
format_change



States description
==================

-----
trans
-----
In this state we do not know output format because we have no enough data to
determine it. So either stream buffer did not catch a syncronization or it was
an error during decoding. Filter is empty in this state and output format is 
unknown.

process() call tries to syncronize, load and decode a new frame.

from state      to state        output format   action          transition description
--------------------------------------------------------------------------------------
trans           trans                                           not enough data
trans           trans                           reset()         not enough data and flushing
trans           full            parser_spk                      frame loaded

-----
empty
-----
In this state stream buffer has only a part of a frame loaded. Filter is empty
and output format equals to the old parser format.

process() tries to load and decode a frame.                   

from state      to state        output format   action          transition description
--------------------------------------------------------------------------------------
empty           empty                                           not enough data
empty           no_frame                                        not enough data and flushing (finish the stream)
empty           full                                            frame loaded
empty           format_change                                   frame loaded belongs to a new stream

----
full
----
Stream buffer has a frame loaded. Filter is full and just do data output. Note,
that we cannot load the stream buffer again because this will corrupt data
returned from this call.

from state      to state        output format   chunk data      transition description
--------------------------------------------------------------------------------------
full            no_frame                        chunk/frame     data output

--------
no_frame
--------
Try to load the stream buffer and do data output. Handle output format changes.

from state      to state        output format   chunk data      transition description
--------------------------------------------------------------------------------------
no_frame        trans           spk_unk         chunk/eos       not enough data and flushing
no_frame        empty                           chunk/dummy     not enough data
no_frame        no_frame                        chunk/frame     send a new frame loaded
no_frame        full            parser_spk      chunk/eos       frame loaded belongs to a new stream  

-------------
format_change
-------------
Send flushing and handle output format change.

from state      to state        output format   chunk data      transition description
--------------------------------------------------------------------------------------
format_change   full            parser_spk      chunk/eos       inter-stream flushing



Transitions list
================

from state      to state        output format   action          transition description
--------------------------------------------------------------------------------------
trans           trans                                           not enough data
trans           trans                           reset()         not enough data and flushing
trans           full            parser_spk                      frame loaded
empty           empty                                           not enough data
empty           no_frame                                        not enough data and flushing (finish the stream)
empty           full                                            frame loaded
empty           format_change                                   frame loaded belongs to a new stream
full            no_frmae                        chunk/frame     data output
no_frame        trans           spk_unk         chunk/eos       not enough data and flushing
no_frame        empty                           chunk/dummy     not enough data
no_frame        no_frame                        chunk/frame     send a new frame loaded
no_frame        full            parser_spk      chunk/eos       frame loaded belongs to a new stream  
format_change   full            parser_spk      chunk/eos       inter-stream flushing

*/


ParserFilter::ParserFilter()
:NullFilter(-1)
{
  parser = 0;
  errors = 0;

  out_spk = spk_unknown;
  state = state_trans;
  new_stream = false;
}

ParserFilter::ParserFilter(FrameParser *_parser)
:NullFilter(-1)
{
  parser = 0;
  errors = 0;

  out_spk = spk_unknown;
  state = state_trans;
  new_stream = false;

  set_parser(_parser);
}

ParserFilter::~ParserFilter()
{
}

bool
ParserFilter::set_parser(FrameParser *_parser)
{
  reset();
  parser = 0;

  if (!_parser)
    return true;

  const HeaderParser *header_parser = _parser->header_parser();
  if (!stream.set_parser(header_parser))
    return false;

  parser = _parser;
  return true;
}

const FrameParser *
ParserFilter::get_parser() const
{
  return parser;
}

size_t
ParserFilter::get_info(char *buf, size_t size) const
{
  char info[2048];
  size_t len = 0;
  
  len += stream.stream_info(info + len, sizeof(info) - len); 
  if (parser)
    len += parser->stream_info(info + len, sizeof(info) - len); 

  if (len + 1 > size) len = size - 1;
  memcpy(buf, info, len + 1);
  buf[len] = 0;
  return len;
}



void
ParserFilter::reset()
{
  NullFilter::reset();

  out_spk = spk_unknown;
  state = state_trans;

  if (parser)
    parser->reset();
  stream.reset();
  sync_helper.reset();
  new_stream = false;
}

bool
ParserFilter::is_ofdd() const
{
  return true;
}

bool
ParserFilter::query_input(Speakers spk) const
{
  if (!parser) 
    return false;
  else if (spk.format == FORMAT_RAWDATA) 
    return true;
  else
    return parser->header_parser()->can_parse(spk.format);
}

bool
ParserFilter::process(const Chunk *_chunk)
{
  assert(is_empty());

  if (!parser)
    return false;

  // we must ignore dummy chunks
  if (_chunk->is_dummy())
    return true;

  // receive the chunk
  FILTER_SAFE(receive_chunk(_chunk));
  sync_helper.receive_sync(_chunk, stream.get_buffer_size());
  sync = false;

  switch (state)
  {
  case state_trans:
    if (load_parse_frame())
    {
      out_spk = parser->get_spk();
      state = state_full;
      new_stream = false;
    }
    else if (flushing)
    {
      // if we did not start a stream we must drop data currently buffered
      // (flushing state is also dropped so we do not pass eos event in this case)

      // it is implied that reset() does following:
      // * spk = spk_unknown;
      // * state = state_trans;
      // * flushing = false
      reset();
    }
    return true;

  case state_empty:
    if (load_parse_frame())
    {
      if (new_stream)
      {
        state = state_format_change;
        new_stream = false;
      }
      else
        state = state_full;
    }
    else if (flushing)
    {
      // if we have started a stream we must finish it correctly by sending
      // end-of-stream chunk (flushing is sent on get_chunk() call).
      state = state_no_frame;
    }
    return true;
  }

  // never be here
  assert(false);
  return false;
}

Speakers
ParserFilter::get_output() const
{
  return out_spk;
}

bool
ParserFilter::is_empty() const
{
  return state == state_empty || state == state_trans;
}

bool
ParserFilter::get_chunk(Chunk *_chunk)
{
  assert(!is_empty());

  if (!parser) 
    return false;

  switch (state)
  {
    case state_full:
      send_frame(_chunk);
      state = state_no_frame;
      return true;

    case state_no_frame:
      // load next frame, parse and send it
      // * send inter-stream flusing if new frame belongs to a new stream
      // * send dummy chunk if we have no enough data to load a frame
      // * send flushing and reset if we have no enough data to load a frame
      //   and have received flushing from upstream before

      if (load_parse_frame())
      {
        if (new_stream)
        {
          // send inter-stream flushing
          send_eos(_chunk);
          out_spk = parser->get_spk();
          state = state_full;
          new_stream = false;
        }
        else
        {
          // send the parsed frame
          send_frame(_chunk);
          state = state_no_frame;
        }
      }
      else // if (load_parse_frame())
      {
        if (flushing)
        {
          // send end-of-stream flushing
          send_eos(_chunk);

          // it is implied that reset() does following:
          // * spk = spk_unknown;
          // * state = state_trans;
          // * flushing = false;
          reset();
        }
        else
        {
          // send dummy
          _chunk->set_dummy();
          state = state_empty;
        }
      }

      return true;

    case state_format_change:
      // send inter-stream flushing
      _chunk->set_empty(out_spk);
      _chunk->set_eos();

      out_spk = parser->get_spk();
      state = state_full;
      return true;

    default:
      return false;
  }
}

bool
ParserFilter::load_parse_frame()
{
  uint8_t *end = rawdata + size;
  size_t old_data_size = size + stream.get_buffer_size();

  while (stream.load_frame(&rawdata, end))
  {
    new_stream |= stream.is_new_stream();
    Speakers old_parser_spk = parser->get_spk();
    if (parser->parse_frame(stream.get_frame(), stream.get_frame_size()))
    {
      if (old_parser_spk != parser->get_spk())
        new_stream = true;
      size = end - rawdata; 
      sync_helper.drop(old_data_size - size - stream.get_buffer_size());
      return true;
    }
    else
      errors++;
  }

  sync_helper.drop(old_data_size - stream.get_buffer_size());
  size = 0;
  return false;
}

void
ParserFilter::send_frame(Chunk *_chunk)
{
  if (out_spk.is_linear())
    _chunk->set_linear(out_spk, parser->get_samples(), parser->get_nsamples());
  else
    _chunk->set_rawdata(out_spk, parser->get_rawdata(), parser->get_rawsize());

  sync_helper.send_frame_sync(_chunk);
}

void
ParserFilter::send_eos(Chunk *_chunk)
{
  _chunk->set_empty(out_spk);
  _chunk->set_eos(true);

  // reset syncronization after the end of the stream
  sync_helper.reset(); 
}
