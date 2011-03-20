#pragma once
#ifndef AUDIOFILTER_MPEGDEMUX_H
#define AUDIOFILTER_MPEGDEMUX_H
/*
 * MPEG parser&demuxer
 *
 * //////////////////////////////////////////////////////////////////////////////
 * // PSParser - a simple MPEG1/2 Program Stream parser.
 *
 * //////////////////////////////////////////////////////////////////////////////
 * // PSDemux - a simple MPEG1/2 Program Stream demuxer.
 *
 * This class is also a good example how to use PSParser class.
 *
 * Demux may work in 2 modes:
 * 1) Extract certain elementary stream from a multiplexed MPEG stream.
 * 2) Extract elementart stream from a PES stream.
 *
 * In first mode we must to know stream/substream numbers we want to extract.
 * Demuxer will extract only the specified stream.
 *
 * In second mode we may not know stream/substream numbers (set it to 0).
 * Demuxer will extract any stream it finds. So input must contain exactly one
 * stream (PES stream).
 *
 * Demuxer interface is extremely simple: we must just specify stream/substream
 * numbers, get stream buffer and call demux() function. It replaces original
 * buffer data with demuxed data. New data size is returned.
 * //////////////////////////////////////////////////////////////////////////////
 *
 * todo: Transport Stream parser
 */

#include "Defs.h"
#include "Speakers.h"
#include "SyncScan.h"

namespace AudioFilter {

class PSDemux;

class PSParser
{
public: // public interface
  PSParser();

  void reset(void);
  size_t parse(uint8_t **buf, uint8_t *end);

  bool isAudio(void);
  Speakers spk(void);

  size_t getHeaderSize(void) const
  {
    return header_size;
  }

  const uint8_t *getHeader(void) const
  {
    return &header[0];
  }

  size_t getPayloadSize(void) const
  {
    return payload_size;
  }

  void subtractFromPayloadSize(size_t n)
  {
    payload_size -= n;
  }

  int getStream(void) const
  {
    return stream;
  }

  int getSubStream(void) const
  {
    return substream;
  }

  int getErrors(void) const
  {
    return errors;
  }

private: // public data
  uint8_t header[268];  // packet header (including substream header)
  uint8_t *subheader;   // pointer to subheader start (0 - no subheader)

  size_t  header_size;  // header size
  size_t  payload_size; // packet payload size;

  int stream;           // stream number
  int substream;        // substream number (0 for no substream)

  int packets;          // packets processed
  int errors;           // errors

private: // private data
  SyncScan scanner;
  enum { state_sync, state_header, state_drop } state;
  size_t data_size;     // data size for internal use

  friend class PSDemux;
};

class PSDemux
{
public: // public interface
  PSDemux(int stream = 0, int substream = 0);

  void reset(void);
  void set(int stream, int substream = 0);
  size_t demux(uint8_t *buf, size_t size);

private: // public data
  PSParser parser;
  int stream;
  int substream;

};

}; // namespace AudioFilter

#endif

// vim: ts=2 sts=2 et

