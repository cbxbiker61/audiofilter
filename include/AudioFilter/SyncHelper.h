#pragma once
#ifndef AUDIOFILTER_SYNCHELPER_H
#define AUDIOFILTER_SYNCHELPER_H
/*
  Sync helper class.
  Received timestamp is for the first syncpoint in the chunk.
*/

#include "Filter.h"

namespace AudioFilter {

///////////////////////////////////////////////////////////////////////////////
// Obsolete, will be removed
// Use SyncHelper instead (below)


class Sync
{
public:
  Sync()
  {
    reset();
  }

  inline void receiveSync(bool _sync, vtime_t _time)
  {
    if ( _sync )
    {
      if ( syncing )
      {
        sync[0] = true;
        time[0] = _time;
        sync[1] = false;
        time[1] = _time;
      }
      else
      {
        sync[1] = true;
        time[1] = _time;
      }
    }
  }

  inline void receiveSync(const Chunk *chunk)
  {
    receiveSync(chunk->sync, chunk->time);
  }

  inline void sendSync(Chunk *_chunk)
  {
    _chunk->sync = sync[0];
    _chunk->time = time[0];
    sync[0] = sync[1];
    time[0] = time[1];
    sync[1] = false;
  }

  inline bool isSyncing(void)
  {
    return syncing;
  }

  inline void setSyncing(bool _syncing)
  {
    syncing = _syncing;
  }

  inline void reset(void)
  {
    syncing = true;
    sync[0] = false;
    sync[1] = false;
  }

protected:
  bool    syncing; // syncing state
  bool    sync[2]; // timestamp exists
  vtime_t time[2]; // timestamp
};

///////////////////////////////////////////////////////////////////////////////
// SyncHelper
//
// Helper class that holds sync information and makes correct timestamps for
// output chunks.
//
// receive_sync(Chunk *chunk, size_t pos)
//   Receive sync info from the input chunk.
//   This syncpoint will be applied to the buffer position pos.
//
// send_frame_sync(Chunk *chunk)
//   Timestamp the output chunk if nessesary.
//   Chunk will be stamped with the first timestamp in the queue.
//   This is applicable when chunks are frames of compressed data and we must
//   apply the timestamp to the first frame after the timestamp received:
//
//
//   t1           t2           t3
//   v            v            v
//   +------------+------------+------------+
//   |   chunk1   |   chunk2   |   chunk3   |
//   +------------+------------+------------+
//
//   +--------+--------+--------+--------+--
//   | frame1 | frame2 | frame3 | frame4 |
//   +--------+--------+--------+--------+--
//   ^        ^        ^        ^
//   t1    no time     t2       t3
//
//
// send_sync(Chunk *chunk, double size_to_time)
//   Timestamp the output chunk if nessesary.
//   Chunk will be stamped with the first timestamp in the queue, shifted by
//   the amount of data between the timestamp received and the chunk's start.
//   Applicable for linear and PCM data:
//
//   t1           t2           t3
//   v            v            v
//   +------------+------------+------------+
//   |   chunk1   |   chunk2   |   chunk3   |
//   +------------+------------+------------+
//
//   +--------+--------+--------+--------+--
//   | chunk1 | chunk2 | chunk3 | chunk4 |
//   +--------+--------+--------+--------+--
//   ^        ^        ^        ^
//   t1    no time     t2+5     t1+1
//
// drop(int size)
//   Drop buffered data. Required to track suncpoint positions.
//
// reset()
//   Drop all timing information.


class SyncHelper
{
public:
  SyncHelper()
  {
    reset();
  }

  inline void receiveSync(const Chunk *chunk, pos_t pos);
  inline void sendFrameSync(Chunk *chunk);
  inline void sendSync(Chunk *chunk, double size_to_time);
  inline void drop(size_t size);
  inline void reset(void);

protected:
  bool    sync[2]; // timestamp exists
  vtime_t time[2]; // timestamp
  pos_t   pos[2];  // buffer position for timestamp

  inline void shift(void);
};

///////////////////////////////////////////////////////////////////////////////

inline void
SyncHelper::shift(void)
{
  sync[0] = sync[1];
  time[0] = time[1];
  pos[0]  = pos[1];
  sync[1] = false;
  time[1] = 0;
  pos[1]  = 0;
}

inline void
SyncHelper::receiveSync(const Chunk *chunk, pos_t _pos)
{
  if ( chunk->sync )
  {
    if ( sync[0] && pos[0] != _pos )
    {
      assert(pos[0] < _pos);
      sync[1] = true;
      time[1] = chunk->time;
      pos[1]  = _pos;
    }
    else
    {
      sync[0] = true;
      time[0] = chunk->time;
      pos[0]  = _pos;
    }
  }
}

inline void
SyncHelper::sendFrameSync(Chunk *chunk)
{
  if ( pos[0] <= 0 )
  {
    chunk->sync = sync[0];
    chunk->time = time[0];
    shift();
  }
}

inline void
SyncHelper::sendSync(Chunk *chunk, double size_to_time)
{
  if ( pos[0] <= 0 )
  {
    chunk->sync = sync[0];
    chunk->time = time[0] - pos[0] * size_to_time;
    shift();
  }
}

inline void
SyncHelper::drop(size_t size)
{
  if ( sync[0] )
    pos[0] -= size;

  if ( sync[1] )
  {
    pos[1] -= size;
    if (pos[1] <= 0)
      shift();
  }
}

inline void
SyncHelper::reset(void)
{
  sync[0] = false;
  time[0] = 0;
  pos[0]  = 0;
  sync[1] = false;
  time[1] = 0;
  pos[1]  = 0;
}

}; // namespace AudioFilter

#endif

// vim: ts=2 sts=2 et

