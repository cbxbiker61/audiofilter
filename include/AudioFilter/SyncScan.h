#pragma once
#ifndef AUDIOFILTER_SYNCSCAN_H
#define AUDIOFILTER_SYNCSCAN_H

/*
  SyncScan
  Syncronization scanner. Searches for 32bit syncronization words. May seach
  many syncwords simultaneously. Speed-optimized.

  -----------------------------------------------------------------------------
  SCANNING ALGORITHM AND SYNCRONIZATION TABLE
  -----------------------------------------------------------------------------

  To find 32bit syncpoint we must ensure that 4 successive bytes belongs to
  the same syncronization point (note that we may seach for different kinds of
  syncpoints simultaneously). To do this we may check it directly:

  uint32_t *ptr;
  ...
  if ((*ptr == sync1) || (*ptr == sync2) || (*ptr == sync3) ... )
  {
    // found
  }
  else
  {
    // not found, try next byte
  }

  This method obviously is not too good. For exmple, MPA syncword contains only
  12 bits of syncword so we must search it as following:

  if (((*ptr & 0x0000fff0) == 0x0000fff0) ||
     ((*ptr & 0x0000fff0) == 0x0000fff0))
  {
    // MPA syncpoint found
  }

  So universal algorithm may look like:

  if (((*ptr & mask1) == sync1) || ((*ptr & mask2) == sync2) ... )
  {
    // found
  }

  To make scanner configurable we must have 2 tables with syncwords and masks
  that must grow with complexity of syncword checking (for example we may want
  to prohibit incorrect params in first 32bits of MPA header).

  Memory access is also not optimal. It requires 2 table accesses per 1
  syncpoint for each position. As will be shown below it may be decreased to
  2 table lookups per 32 syncpoints for each position. Syncpoints may be more
  than just a syncword allowing more comprehensive (and effective) scanning.

  So let's consider table lookup method. To ensure that 4 bytes belongs to the
  same syncpoint with table lookup we may write following:

  if ((table[b1] == table[b2]) &&
      (table[b2] == table[b3]) &&
      (table[b3] == table[b4]))
  {
    // found
  }

  Obvious weakness of this algorithm is one table: we may not distinguish
  sequences 0xff 0x1f 0x00 0xe8 and 0x1f 0xff 0xe8 0x00 (DTS big and low
  endian) and 0xff 0x00 0x00 0x00 (not a syncpoint at all).

  if ((table1[b1] == table2[b2]) &&
      (table2[b2] == table3[b3]) &&
      (table3[b3] == table4[b4]))
  {
    // found
  }

  This one is better but here we cannot simultaneously search syncpoints
  that share the same byte: 0xff 0xfn (MPA) and 0xff 0x1f 0x00 0xe8 (DTS).
  And we have excessive table accesses here (1.5 per byte).

  We may solve all problems if we build table not with syncpoint NUMBERS
  but with syncpoint MASKS and do not compare but do bit operations.
  So syncpoint check will look like:

  if (t1[b1] & t2[b2] & t3[b3] & t4[b4])
  {
    // found
  }

  Here if the same bit set in all tables (all bytes belong to the same
  syncpoint) we have non-zero result and this bit identifies kind of the
  syncpoint.

  But it is not all. Because syncronization points are usually built to
  decrease probability of false-sync we can decrease number of table accesses:

  if (t1[b1])
  if (t1[b1] & t2[b2] & t3[b3] & t4[b4])
  {
    // found
  }

  Because of small probability that b1 belongs to a syncpoint we may not check
  all other bytes. Is it good? NO!

  The probability of that b1 belongs to a syncpoint may be up to 1/16
  (for MPA). You may say that is good enough but it is not! Because
  branch-prediction algorithm of modern processors begins to fail and most of
  efficiency gain is compensated by negative effect of mispredicted branches.

  The solution is to decrease probability of first condition fulfilment:

  if (t1[b1] & t2[b2])
  if (t1[b1] & t2[b2] & t3[b3] & t4[b4])
  {
    // found
  }

  This increases number of table lookups but decreases probability of
  mispredicted branch to about 1/4000 (MPA). This algorithm shows much higher
  performance. Also with 32bit lookup table we may search up to 32 types of
  syncpoints simultaneousely with only 2.00025 table lookups at each position
  (1/16 table accesses per syncpoint).

  Ok, what kinds of constraints checking we may do here? Let's consider
  MPA header:

  12bits - syncword (0xfff)
  1bit   - MPEG1/2 flag
  2bits  - Layer
  1bit   - proptection bit
  ...

  So first byte is always 0xff. Layer field in the second byte may be equal to:
  11b - Layer1, 10b - Layer2, 01b - Layer3. Note that value of 00b is
  prohibited. Can we check it with our table lookup algorithm? Yes, we can!
  Prohibited values of the second byte are: 0xf0, 0xf1, 0xf8 and 0xf9. So we
  may clear MPA bit for this values in the table2 and set it for all other
  values. So syncpoint in our case is not just a syncword and a mask but a
  list of possible values.

  This method is not possible for direct comparison: we have either to check
  each possible syncpoint value (252 items in example above) or have a table
  of prohibited values (and masks...) or build direct equation (but it is hard
  to make scanner fast and configurable in this case).

  But table method also has restrications: we cannot analyze conditions
  between bytes. For example if one bit of 'Layer' field belongs to one byte
  and another bit to another byte we cannot perform this constraint check!

  Capacity of table is fixed to bit-depth of table elements and table is bigger
  that just a list of syncpoints. If we need to scan for 1 syncpoint direct
  comparison is faster (but it depends on implementation, because simple way
  may be slower than scanner usage).

  -----------------------------------------------------------------------------
  HOW TO BUILD A SYNCRONIZATION TABLE
  -----------------------------------------------------------------------------

  1) If we want just to find a syncword: use set() or set_list() with required
     syncword ot list of syncwords.

  2) If we need effective sync with more complex conditions we must prepare
     all possible values for each byte. Examples:
     a) MPEG Program Stream has fixed syncword 0x00 0x00 0x01 followed by
        a stream number. Allowed stream number are > 0xb8.
     b) AC3 has 2 syncwords: 0x0b 0x77 and 0x77 0x0b (big and low endian).
     c) MPA has fixed syncword 0xfff0 followed by header that has following
        constraints: layer != 0 (prohibited values for 2nd byte: 0xf0, 0xf1,
        0xf8 and 0xf9), bitrate index != 0xf (prohibited values for 3rd
        byte are 0xfn) and sample rate index != 3 (prohibited values for 3rd
        byte are 0xnc, 0xnd, 0xne, oxnf) where n - any digit.

     Set syncpoint bit to all allowed bytes. But we must remember that allowed
     bytes may appear in any combination:

     a) allow 0 value for 1st byte, 2nd byte, value of 1 to 3rd byte and
        all values > 0xb8 for 4th byte. All other values must have format bit
        clear:

        scan.set(1, 0x000001b9, 0xffffffff); // only b9 stream is allowed now

        // allow other streams
        for (int stream = 0xb9; stream < 0xff; stream++)
          scan.allow(1, stream, 0x000000ff); // allow value 'stream'

     b) Though we have 2 values allowed for 1st and 2nd byte (0x0b and 0x77)
        we must not allow it for the same syncpoint because we may catch
        0x0b 0x0b or 0x77 0x77 'syncpoint' that is not what we want. Therefore
        we must init 2 different syncpoints:

        scan.set(1, 0x770b0000, 0xffff0000);
        scan.set(2, 0x0b770000, 0xffff0000);

     c) Much of conditions. But all conditions are independent: if we see any
        of the bytes prohibited we do not sync. So synctable initialization
        looks complicated but really simple (counting zeros is the most complex
        task here :)

        scan.set(1, 0xfff00000, 0xfff00000); // allow anything after syncword
        scan.deny(1, 0x00000000, 0x00060000); // deny value of 00 for layer
        scan.deny(1, 0x0000f000, 0x0000f000); // deny value of 11 for bitrate
        scan.deny(1, 0x00000c00, 0x00000c00); // deny value of 11 for samplerate

  -----------------------------------------------------------------------------
  HOW TO USE SCANNER CLASS
  -----------------------------------------------------------------------------

  Usually scanning is done on block-basis: we load one block, scan it, load
  another, scan, etc. So to detect sync that may appear at between of blocks
  (one byte of syncpoint belongs to one block and others to another block) we
  need a buffer to store intermediate result in between of calls to scanner.

  SyncScan class supports 2 modes of operation:
  1) using internal buffer
  2) using external buffer

  Internal buffer mode
  ====================

  Internal buffer simplifies interface and class usage when we need just to
  detect a syncpoint. We're not required to provide a buffer, worry about init,
  cleanup, etc. Internal buffer interface:

  syncbuf[]
    Buffer that contains data currently loaded

  count
    Number of bytes currently loaded at buffer

  reset()
    Reset internal buffer to prepare to scan a new stream. For example
    we scan a file that have last byte equal to 0x0b. After that we start
    scanning next file with first byte equal to 0x77. If we do not clear
    internal buffer we'll get false sync that appears _before_ the start of
    new file.

  uint32_t get_sync() const
    Checks internal buffer and report 0 if it is not a syncpoint and syncpoint
    mask otherwise. (Because different syncpoints may have the same syncword
    several bits may be set in the returned value.)

  size_t scan(uint8_t *buf, size_t size)
    Scan a block of size 'size' pointed by 'buf'. Returns number of bytes
    processed. If scan was not succeeded all bytes are always processed
    (returns size). Successful scan may return number of bytes less or equal
    to 'size'. So position of syncpoint after successful scan may be
    calculated as:

      gone = sync.scan(buf, size);
      if (sync.get_sync())
        syncpos = buf + gone - sync.count;

    where
      'gone' - number of bytes returned by scan() function
      'count' - number of bytes at internal buffer (always 4 on success)

    Note that syncpoint position may lay _before_ the buffer start because of
    buffering.

  External buffer mode
  ====================

  External buffer is convenient when we load a data frame that begins with
  syncpoint found. We're not required to copy data to/from scanner buffer.
  This mode does not use 'syncbuf' and 'count' fields and we must carry about
  our buffer init to avoid false-sync. For example buffer inited by zeros
  is not good choice for MPEG Program Stream. So best way to proceed is to
  fill buffer with at least 4 bytes of input data and scan the rest.

  uint32_t get_sync(uint8_t *syncbuf) const
    Fully analogous to get_sync() of internal buffer mode but works with
    external sync buffer.

  size_t scan(uint8_t *syncbuf, uint8_t *buf, size_t size) const
    Fully analogous to scan() of internal buffer mode but works with
    external sync buffer.


  Examples
  ========

  1) Internal buffer mode, just counting syncpoints:

  uint8_t *buf;
  size_t size;
  SyncScan scanner;
  int syncpoints = 0;

  // init synctable
  ...

  // start scan
  scanner.reset();
  while (!have_more_data())
  {
    size = load_buffer(buf);
    size_t gone = scanner.scan(buf, size)
    if (scanner.get_sync())
      syncpoints++;
  }

  1) External buffer mode, counting syncpoints:

  uint8_t *buf;
  size_t size;
  SyncScan scanner;
  uint8_t scanbuf[4];
  int syncpoints = 0;

  // init synctable
  ...
  // load scanbuf
  ...

  // start scan
  scanner.reset();
  while (!have_more_data())
  {
    size = load_buffer(buf);
    size_t gone = scanner.scan(scanbuf, buf, size)
    if (scanner.get_sync(scanbuf))
      syncpoints++;
  }

  3) External buffer mode, recommended frame load algorithm.

  We may have some data loaded into frame_buf so we need to scan frame buffer
  before scanning of new data.

  uint8_t *input_buf;   // input data
  size_t   input_size;  // input data size

  uint8_t *frame_buf;   // frame buffer we load frame to
  size_t frame_data;    // frame buffer data size

  SyncScan scanner;     // scanner
  ...

  /////////////////////////////////////////////////////////
  // Ensure that we have at least 4 bytes of input data
  // loaded into frame buffer

  if (frame_data < 4)
  {
    size_t len = min(input_size, 4);
    memcpy(frame_buf + frame_data, input_buf, len);
    input_buf += len;
    input_size -= len;
    if (frame_data < 4)
    {
      // no syncpoint
    }
  }

  /////////////////////////////////////////////////////////
  // Scan

  if (!scanner.get_sync(frame_buf))
  {
    // scan frame buffer and drop data before syncpoint (if it is)
    size_t gone = scanner.scan(frame_buf, frame_buf + 4, frame_data - 4);
    frame_data -= gone;
    memmove(frame_buf + 4, frame_buf + 4 + gone, frame_data);

    // if we did not find syncpoint at frame buffer scan input data
    if (!scanner.get_sync(frame_buf))
    {
      gone = scanner.scan(frame_buf, input_buf, input_size);
      input_buf  += gone;
      input_size -= gone;
      if (!scanner.get_sync(frame_buf))
      {
        // no syncpoint
      }
    }
  }

  /////////////////////////////////////////////////////////
  // Load the rest of the frame, check conditions, CRC, etc

  ...

  -----------------------------------------------------------------------------
  SYNCPOINTS MAP
  -----------------------------------------------------------------------------

  This module provides 'standard' syncpoints most used in this library: AC3,
  DTS, MPA. But because of limited number of syncpoints scanned simultaneously
  syncpoint indexes are divided into several categories. Of course, any
  syncpoint index may be used but if we need to use standard syncpoints we must
  use only user-definable syncpoints:

  0-7   custom syncpoints (user-definable)
  8     MPA big endian
  9     MPA low endian
  10    AC3 big endian
  11    AC3 low endian
  12    DTS16 big endian
  13    DTS16 low endian
  14    DTS14 big endian
  15    DTS14 low endian
  16-31 reserved (planned: SPDIF syncpoints, MPEG Program Stream)

  Standard syncpoints are defined with most complex error checking possible,
  so it is preferrable to use functionality provided by this module rather than
  define own syncpoints.
*/

#include "Defs.h"

namespace AudioFilter {

///////////////////////////////////////////////////////////
// Syncpoint indexes

#define SYNC_MPA_BE   8
#define SYNC_MPA_LE   9

#define SYNC_AC3_BE   10
#define SYNC_AC3_LE   11

#define SYNC_DTS16_BE 12
#define SYNC_DTS16_LE 13
#define SYNC_DTS14_BE 14
#define SYNC_DTS14_LE 15

#define SYNC_SPDIF    16
#define SYNC_PS       17

///////////////////////////////////////////////////////////
// Syncpoint masks

#define SYNCMASK_MAD      0xff00

#define SYNCMASK_MPA      0x0300
#define SYNCMASK_MPA_BE   0x0100
#define SYNCMASK_MPA_LE   0x0200

#define SYNCMASK_AC3      0x0c00
#define SYNCMASK_AC3_BE   0x0400
#define SYNCMASK_AC3_LE   0x0800

#define SYNCMASK_DTS      0xf000
#define SYNCMASK_DTS16_BE 0x1000
#define SYNCMASK_DTS16_LE 0x2000
#define SYNCMASK_DTS14_BE 0x4000
#define SYNCMASK_DTS14_LE 0x8000

#define SYNCMASK_SPDIF    0x10000
#define SYNCMASK_PS       0x20000

///////////////////////////////////////////////////////////
// SyncScan class

class SyncScan
{
public:
  union
  {
    uint8_t  syncbuf[4];
    uint32_t syncword;
  };
  size_t count;

  SyncScan(uint32_t syncword = 0, uint32_t syncmask = 0);
  ~SyncScan();

  /////////////////////////////////////////////////////////
  // Table operations

  bool   set  (int index, uint32_t syncword, uint32_t syncmask);
  bool   allow(int index, uint32_t syncword, uint32_t syncmask);
  bool   deny (int index, uint32_t syncword, uint32_t syncmask);

  bool   clear(int index);
  void   clearAll(void);

  bool   setList(const uint32_t *_list, unsigned _size);
  void   setStandard(uint32_t syncmask);

  /////////////////////////////////////////////////////////
  // Internal buffer mode interface

  void reset(void);
  uint32_t getSync(void) const;
  size_t scan(uint8_t *buf, size_t size);

  /////////////////////////////////////////////////////////
  // External buffer mode interface

  uint32_t getSync(uint8_t *buf) const;
  size_t scan(uint8_t *syncword, uint8_t *buf, size_t size) const;

private:
  typedef uint32_t synctbl_t;
  synctbl_t *synctable;

};

}; // namespace AudioFilter

#endif

// vim: ts=2 sts=2 et

