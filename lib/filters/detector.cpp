/*

This filter is data-dependent. Therefore it must follow format change rules
carefully.

This implementation is based on the same ideas as ParserFilter (uses
StreamBuffer in a similar way). But to handle output of both raw data and
packetized stream data and handle switches between these modes correctly
it uses 2 state cycles (frame cycle and debris cycle). So this filter is
much more complex thing (31 state transition vs 13 transitions) and to know
how it works it is recommended to learn ParserFilter first.



States list
===========

trans
debris_empty
frame_empty
debris
no_debris
frame_debris
frame
no_frame
format_change



State transitions
=================

-----
trans
-----
In this state we do not know output format because we have no enough data to
determine it (stream buffer did not catch a syncronization). Filter is empty
in this state and output format is unknown.

process() call tries to syncronize and switch to data output state.

from state      to state        output format   action          transition description
--------------------------------------------------------------------------------------
trans           trans                                           not enough data
trans           trans                           reset()         not enough data and flushing
trans           debris          in_spk                          some debris to output
trans           frame           parser_spk                      successful sync

------------
debris_empty
------------
Empty state in debris cycle. Try to load the stream buffer and switch to
appropriate state. Handle output format changes.

from state      to state        output format   action          transition description
--------------------------------------------------------------------------------------
debris_empty    debris_empty                                    not enough data
debris_empty    no_debris                                       not enough data and flushing (finish the stream)
debris_empty    debris                                          more debris was loaded
debris_empty    format_change                                   successful sync

-----------
frame_empty
-----------
Empty state in frame cycle. Try to load the stream buffer and switch to
appropriate state. Handle output format changes.

from state      to state        output format   action          transition description
--------------------------------------------------------------------------------------
frame_empty     frame_empty                                     not enough data
frame_empty     no_frame                                        not enough data and flushing (finish the stream)
frame_empty     frame_debris                                    next frame was loaded with debris
frame_empty     frame                                           next frame was loaded without debris
frame_empty     format_change                                   next frame loaded belongs to a new stream
frame_empty     format_change                                   sync lost

------
debris
------
Debris output state. Just do data ouptut and switch to no_debris state. Note,
that we cannot load the stream buffer again because this will corrupt data
returned from this call.

from state      to state        output format   chunk data      transition description
--------------------------------------------------------------------------------------
debris          no_debris                       chunk/debris    data output

---------
no_debris
---------
Try to load the stream buffer and do data output. Handle output format changes.

from state      to state        output format   chunk data      transition description
--------------------------------------------------------------------------------------
no_debris       debris_empty                    chunk/dummy     not enough data
no_debris       trans           spk_unk         chunk/eos       not enough data and flushing
no_debris       no_debris                       chunk/debris    more debris was loaded
no_debris       frame           parser_spk      chunk/eos       successful sync

------------
frame_debris
------------
Debris output state (frame was loaded with debris). Just do data output and
switch to frame output state. Note, that we cannot load the stream buffer again
because this will corrupt data returned from this call.

from state      to state        output format   chunk data      transition description
--------------------------------------------------------------------------------------
frame_debris    frame                           chunk/debris    data output

-----
frame
-----
Frame output state. Just do data ouptut and switch to no_frame state. Note,
that we cannot load the stream buffer again because this will corrupt data
returned from this call.

from state      to state        output format   chunk data      transition description
--------------------------------------------------------------------------------------
frame           no_frame                        chunk/frame     data output

--------
no_frame
--------
Try to load the stream buffer and do data output. Handle output format changes.

from state      to state        output format   chunk data      transition description
--------------------------------------------------------------------------------------
no_frame        frame_empty                     chunk/dummy     not enough data
no_frame        trans           spk_unk         chunk/eos       not enough data and flushing
no_frame        no_frame                        chunk/frame     next frame was loaded without debris
no_frame        frame                           chunk/debris    next frame was loaded with debris
no_frame        frame           parser_spk      chunk/eos       next frame loaded belongs to a new stream
no_frame        debris          in_spk          chunk/eos       sync lost and debris was loaded
no_frame        trans           spk_unk         chunk/eos       sync lost and no debris was loaded

-------------
format_change
-------------
Send flushing and handle output format change.

from state      to state        output format   chunk data      transition description
--------------------------------------------------------------------------------------
format_change   trans           spk_unk         chunk/eos       Stream buffer is out of sync and does not contain debris
format_change   debris          in_spk          chunk/eos       Stream buffer is out of sync and contains debris
format_change   frame           parser_spk      chunk/eos       Stream buffer is in sync and contains a frame



Full state transitions list
===========================

from state      to state        output format   action          transition description
--------------------------------------------------------------------------------------
trans           trans                                           not enough data
trans           trans                           reset()         not enough data and flushing
trans           debris          in_spk                          some debris to output
trans           frame           parser_spk                      successful sync
debris_empty    debris_empty                                    not enough data
debris_empty    no_debris                                       not enough data and flushing (finish the stream)
debris_empty    debris                                          more debris was loaded
debris_empty    format_change                                   successful sync
frame_empty     frame_empty                                     not enough data
frame_empty     no_frame                                        not enough data and flushing (finish the stream)
frame_empty     frame_debris                                    next frame was loaded with debris
frame_empty     frame                                           next frame was loaded without debris
frame_empty     format_change                                   next frame loaded belongs to a new stream
frame_empty     format_change                                   sync lost
debris          no_debris                       chunk/debris    data output
no_debris       debris_empty                    chunk/dummy     not enough data
no_debris       trans           spk_unk         chunk/eos       not enough data and flushing
no_debris       no_debris                       chunk/debris    more debris was loaded
no_debris       frame           parser_spk      chunk/eos       successful sync
frame_debris    frame                           chunk/debris    data output
frame           no_frame                        chunk/frame     data output
no_frame        frame_empty                     chunk/dummy     not enough data
no_frame        trans           spk_unk         chunk/eos       not enough data and flushing
no_frame        no_frame                        chunk/frame     next frame was loaded without debris
no_frame        frame                           chunk/debris    next frame was loaded with debris
no_frame        frame           parser_spk      chunk/eos       next frame loaded belongs to a new stream
no_frame        debris          in_spk          chunk/eos       sync lost and debris was loaded
no_frame        trans           spk_unk         chunk/eos       sync lost and no debris was loaded
format_change   trans           spk_unk         chunk/eos       Stream buffer is out of sync and does not contain debris
format_change   debris          in_spk          chunk/eos       Stream buffer is out of sync and contains debris
format_change   frame           parser_spk      chunk/eos       Stream buffer is in sync and contains a frame

*/


#include <stdio.h>
#include "detector.h"
#include "../parsers/spdif/spdif_header.h"
#include "../parsers/mpa/mpa_header.h"
#include "../parsers/ac3/ac3_header.h"
#include "../parsers/dts/dts_header.h"


static const HeaderParser *spdif_dts_parsers[] =
{
  &spdif_header,
  &dts_header
};

static const HeaderParser *uni_parsers[] =
{
  &spdif_header,
  &ac3_header,
  &dts_header,
  &mpa_header
};


Detector::Detector()
:NullFilter(-1)
{
  out_spk = spk_unknown;
  state = state_trans;

  spdif_dts_header.set_parsers(spdif_dts_parsers, array_size(spdif_dts_parsers));
  uni_header.set_parsers(uni_parsers, array_size(uni_parsers));
}

Detector::~Detector()
{
}

const HeaderParser *
Detector::find_parser(Speakers spk) const
{
  switch (spk.format)
  {
    case FORMAT_RAWDATA: return &uni_header;
    case FORMAT_PCM16:   return &spdif_dts_header;
    case FORMAT_SPDIF:   return &spdif_dts_header;

    case FORMAT_AC3:     return &ac3_header;
    case FORMAT_DTS:     return &dts_header;
    case FORMAT_MPA:     return &mpa_header;
  };

  return 0;
}

void
Detector::reset()
{
  NullFilter::reset();

  out_spk = spk_unknown;
  state = state_trans;

  stream.reset();
  sync_helper.reset();
}

bool
Detector::is_ofdd() const
{
  return true;
}

bool
Detector::query_input(Speakers _spk) const
{
  return find_parser(_spk) != 0;
}

bool
Detector::set_input(Speakers _spk)
{
  reset();

  const HeaderParser *hparser = find_parser(_spk);
  if (!hparser)
    return false;

  if (!stream.set_parser(hparser))
    return false;

  spk = _spk;
  return true;
}


bool
Detector::process(const Chunk *_chunk)
{
  assert(is_empty());

  // we must ignore dummy chunks
  if (_chunk->is_dummy())
    return true;

  // receive the chunk
  FILTER_SAFE(receive_chunk(_chunk));
  sync_helper.receive_sync(sync, time);
  sync = false;

  if (load())
  {
    switch (state)
    {
    case state_trans:
      if (stream.is_in_sync())
      {
        // successful sync
        out_spk = stream.get_spk();
        state = state_frame;
      }
      else
      {
        // debris output
        out_spk = spk;
        state = state_debris;
      }

      return true;

    case state_empty_debris:
      if (stream.is_in_sync())
        // successful sync
        state = state_format_change;
      else
        // more debris
        state = state_debris;

      return true;

    case state_empty_frame:
      if (stream.is_in_sync())
      {
        if (stream.is_new_stream())
          // new stream
          state = state_format_change;
        else if (stream.is_debris_exists())
          // frame with debris
          state = state_frame_debris;
        else
          // frame without debris
          state = state_frame;
      }
      else
        // sync lost
        state = state_format_change;

      return true;
    }

    // never be here
    assert(false);
    return false;
  }
  else if (flushing)
  {
    switch (state)
    {
    case state_trans:
      // if we did not start a stream we must forget about current stream on 
      // flushing and drop data currently buffered (flushing state is also
      // dropped so we do not pass eos event in this case)

      // it is implied that reset() does following:
      // * spk = spk_unknown;
      // * state = state_trans;
      // * flushing = false;
      reset();
      return true;

    case state_empty_debris:
      // if we have started a stream we must finish it correctly by sending
      // end-of-stream chunk (flushing is sent on get_chunk() call).
      state = state_no_debris;
      return true;

    case state_empty_frame:
      // if we have started a stream we must finish it correctly by sending
      // end-of-stream chunk (flushing is sent on get_chunk() call).
      state = state_no_frame;
      return true;
    }

    // never be here
    assert(false);
    return false;
  }
  else
    // not enough data to load stream buffer (do nothing)
    return true;
}

Speakers
Detector::get_output() const
{
  return out_spk;
}

bool
Detector::is_empty() const
{
  return state == state_trans || state == state_empty_debris || state == state_empty_frame;
}

bool
Detector::get_chunk(Chunk *_chunk)
{
  assert(!is_empty());

  switch (state)
  {
    ///////////////////////////////////////////////////////////////////////////
    // Debris cycle
    // state_debris -> state_no_debris -> state_debris_empty => state_debris

    case state_debris:
      send_debris(_chunk);
      state = state_no_debris;
      return true;

    case state_no_debris:
      // try to load the stream buffer and send data
      // * send debris if we did not catch a sync
      // * send inter-stream flushing and switch output format on successful sync
      // * send dummy chunk if we have no enough data to load the stream buffer
      // * send flushing and reset if we have no enough data to load the stream
      //   buffer and have received flushing from upstream before

      if (load())
      {
        if (stream.is_in_sync())
        {
          send_eos(_chunk);
          out_spk = stream.get_spk();
          state = state_frame;
        }
        else
        {
          send_debris(_chunk);
          state = state_no_debris;
        }
      }
      else // if (load_frame())
      {
        if (flushing)
        {
          // Now we may have some data stuck at the stream buffer. To maintain
          // bit-by-bit correctness we must release this data and send it with
          // the last chunk.
          stream.flush();
          _chunk->set_rawdata(out_spk, stream.get_debris(), stream.get_debris_size());
          _chunk->set_eos();
          sync_helper.send_sync(_chunk);

          // it is implied that reset() does following:
          // * spk = spk_unknown;
          // * state = state_trans;
          // * flushing = false;
          reset();
        }
        else
        {
          _chunk->set_dummy();
          state = state_empty_debris;
        }
      }
      return true;

    ///////////////////////////////////////////////////////////////////////////
    // Frame cycle
    // state_frame_debris -> state_frame -> state_no_frame -> state_frame_empty

    case state_frame_debris:
      send_frame_debris(_chunk);
      state = state_frame;
      return true;

    case state_frame:
      send_frame(_chunk);
      state = state_no_frame;
      return true;

    case state_no_frame:
      // try to load the stream buffer and send data
      // * send inter-stream flushing and switch output format on new stream
      // * send dummy chunk if we have no enough data to load the stream buffer
      // * send flushing and reset if we have no enough data to load the stream
      //   buffer and have received flushing from upstream before
      // * send debris if we did not catch a sync

      if (load())
      {
        if (stream.is_in_sync())
        {
          if (stream.is_new_stream())
          {
            send_eos(_chunk);
            out_spk = stream.get_spk();
            state = state_frame;
          }
          else
          {
            if (stream.is_debris_exists())
            {
              send_frame_debris(_chunk);
              state = state_frame;
            }
            else
            {
              send_frame(_chunk);
              state = state_no_frame;
            }
          }
        }
        else // if (stream.is_in_sync())
        {
          // inter-stream flushing
          send_eos(_chunk);

          if (stream.is_debris_exists())
          {
            // switch to debris output state
            out_spk = spk;
            state = state_debris;
          }
          else
          {
            // switch to transition state
            out_spk = spk_unknown;
            state = state_trans;
          }
        }
      }
      else // if (load())
      {
        if (flushing)
        {
          send_eos(_chunk);
          // it is implied that reset() does following:
          // * spk = spk_unknown;
          // * state = state_trans;
          // * flushing = false;
          reset();
        }
        else
        {
          _chunk->set_dummy();
          state = state_empty_frame;
        }
      }
      return true;

    ///////////////////////////////////////////////////////////////////////////
    // Format chage

    case state_format_change:
      send_eos(_chunk);

      if (stream.is_in_sync())
      {
        out_spk = stream.get_spk();
        if (stream.is_debris_exists())
          state = state_frame_debris;
        else
          state = state_frame;
      }
      else if (stream.is_debris_exists())
      {
        out_spk = spk;
        state = state_debris;
      }
      else
      {
        out_spk = spk_unknown;
        state = state_trans;
      }
      return true;
  }

  // never be here
  assert(false);
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// Support functions

bool
Detector::load()
{
  uint8_t *end = rawdata + size;
  bool result = stream.load(&rawdata, end);
  size = end - rawdata;
  return result;
}

void
Detector::send_debris(Chunk *_chunk)
{
  _chunk->set_rawdata(out_spk, stream.get_debris(), stream.get_debris_size());
  sync_helper.send_sync(_chunk);
}

void
Detector::send_frame_debris(Chunk *_chunk)
{
  _chunk->set_rawdata(out_spk, stream.get_debris(), stream.get_debris_size());
  // no syncronization sent for inter-stream debris
}

void
Detector::send_frame(Chunk *_chunk)
{
  _chunk->set_rawdata(out_spk, stream.get_frame(), stream.get_frame_size());
  sync_helper.send_sync(_chunk);
}

void
Detector::send_eos(Chunk *_chunk)
{
  _chunk->set_empty(out_spk);
  _chunk->set_eos(true);

  // reset syncronization after the end of the stream
  sync_helper.reset(); 
}
