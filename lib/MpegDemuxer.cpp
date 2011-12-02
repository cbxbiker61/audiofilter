#include <cstring>
#include <AudioFilter/MpegDemux.h>

namespace AudioFilter {

// todo: check marker bits

///////////////////////////////////////////////////////////////////////////////
// PSDemux
///////////////////////////////////////////////////////////////////////////////

PSDemux::PSDemux(int _stream, int _substream)
{
  stream = _stream;
  substream = _substream;
}

void PSDemux::reset(void)
{
  parser.reset();
}

void PSDemux::set(int _stream, int _substream)
{
  stream = _stream;
  substream = _substream;
  parser.reset();
}

size_t PSDemux::demux(uint8_t *_buf, size_t _size)
{
  uint8_t *end = _buf + _size;
  uint8_t *read_buf  = _buf;
  uint8_t *write_buf = _buf;
  size_t len;

  while ( read_buf < end )
  {
    if ( parser.payload_size )
    {
      len = MIN(parser.payload_size, size_t(end - read_buf));

      // drop other streams
      if ( (stream && stream != parser.stream)
          || (stream && substream && substream != parser.substream) )
      {
        parser.payload_size -= len;
        read_buf += len;
        continue;
      }

      // demux
      memmove(write_buf, read_buf, len);
      parser.payload_size -= len;
      read_buf += len;
      write_buf += len;
    }
    else
      parser.parse(&read_buf, end);
  }

  return write_buf - _buf;
}

///////////////////////////////////////////////////////////////////////////////
// PSParser
///////////////////////////////////////////////////////////////////////////////

PSParser::PSParser()
{
  packets = 0;
  errors = 0;
  reset();
}

void PSParser::reset(void)
{
  scanner.setStandard(SYNCMASK_PS);
  state = state_sync;
  data_size = 0;

  subheader = 0;

  header_size = 0;
  payload_size = 0;

  stream = 0;
  substream = 0;
}

bool PSParser::isAudio(void)
{
  return (((stream    & 0xe0) == 0xc0) ||   // MPEG audio stream
          ((substream & 0xf8) == 0x80) ||   // AC3 audio substream
          ((substream & 0xf8) == 0x88) ||   // DTS audio substream
          ((substream & 0xf8) == 0xA0));    // LPCM audio substream
}

Speakers PSParser::spk(void)
{
  // convert LPCM number of channels to channel mask
  static const int nch2mask[8] =
  {
    MODE_MONO,
    MODE_STEREO,
    MODE_3_1,
    MODE_QUADRO,
    MODE_3_2,
    MODE_5_1,
    0, 0
  };

  if ( (stream & 0xe0) == 0xc0 ) // MPEG audio stream
    return Speakers(FORMAT_MPA, 0, 0);
  else if ( (substream & 0xf8) == 0x80 ) // AC3 audio substream
    return Speakers(FORMAT_AC3, 0, 0);
  else if ( (substream & 0xf8) == 0x88 ) // DTS audio substream
    return Speakers(FORMAT_DTS, 0, 0);
  else if ( (substream & 0xf8) == 0xA0 ) // LPCM audio substream
  {
    // parse LPCM header
    // note that MPEG LPCM uses big-endian format
    int format, mask, sample_rate;

    switch ( subheader[4] >> 6 )
    {
      case 0: format = FORMAT_PCM16_BE; break;
      case 1: format = FORMAT_LPCM20;   break;
      case 2: format = FORMAT_LPCM24;   break;
      default: return Speakers::UNKNOWN;
    }

    mask = nch2mask[subheader[4] & 7];

    if ( ! mask )
      return Speakers::UNKNOWN;

    switch ( (subheader[4] >> 4) & 3 )
    {
      case 0: sample_rate = 48000; break;
      case 1: sample_rate = 96000; break;
      default: return Speakers::UNKNOWN;
    }

    return Speakers(format, mask, sample_rate);
  }
  else
    // not an audio format
    return Speakers::UNKNOWN;
}

size_t PSParser::parse(uint8_t **buf, uint8_t *end)
{
  size_t len;
  size_t required_size = 0;

  #define REQUIRE(bytes)       \
  if (data_size < (bytes))     \
  {                            \
    required_size = (bytes);   \
    continue;                  \
  }

  #define DROP                 \
  {                            \
    REQUIRE(6);                \
    data_size = 6 + (header[4] << 8) + header[5] - data_size; \
    state = state_drop;        \
    continue;                  \
  }

  #define SYNC                 \
  {                            \
    data_size = 0;             \
    state = state_sync;        \
    required_size = 4;         \
    continue;                  \
  }

  #define RESYNC(bytes)        \
  {                            \
    data_size -= bytes;        \
    memmove(header, header + bytes, data_size); \
    state = state_sync;        \
    required_size = 4;         \
    continue;                  \
  }

  ///////////////////////////////////////////////////////////////////
  // Forget about previous packet
  // Do not zap stream/substream because it may be used for pes demux

  header_size = 0;
  payload_size = 0;

  for ( ; ; )
  {
    /////////////////////////////////////////////////////////////////
    // Load header data

    if ( data_size < required_size )
    {
      len = MIN(size_t(end - *buf), required_size - data_size);
      memcpy(header + data_size, *buf, len);
      data_size += len;
      *buf += len;

      if ( data_size < required_size )
        return 0;
    }

    switch ( state )
    {
      /////////////////////////////////////////////////////////////////
      // Drop unneeded data
      // data_size - size of data to drop

      case state_drop:
      {
        len = MIN(size_t(end - *buf), data_size);
        data_size -= len;
        *buf += len;

        if ( data_size )
          return 0;
        else
          SYNC;
      } // case state_drop:

      /////////////////////////////////////////////////////////////////
      // Sync
      // data_size - size of data at header buffer

      case state_sync:
      {
        REQUIRE(4);

        if ( ! scanner.getSync(header) )
        {
          size_t gone = scanner.scan(header, header + 4, data_size - 4);
          data_size -= gone;
          memmove(header + 4, header + 4 + gone, data_size);

          if ( ! scanner.getSync(header) )
          {
            gone = scanner.scan(header, *buf, end - *buf);
            *buf += gone;

            if ( ! scanner.getSync(header) )
              return 0;
          }
        }

        // syncword
        if ( header[0] != 0 || header[1] != 0 || header[2] != 1 )
          RESYNC(1);

        // check for invalid stream number (false sync)
        if ( header[3] < 0xb9 )
          RESYNC(1);

        // ignore program end code
        if ( header[3] == 0xb9 )
          RESYNC(4);

        state = state_header;
        // no break: now we go to state_header
      } // case demux_sync:

      /////////////////////////////////////////////////////////////////
      // Parse header
      // data_size - size of data at header buffer

      case state_header:
      {
        /////////////////////////////////////////////////////////////////
        // Drop unused packets

        switch ( header[3] )
        {
          case 0xba: // pack header
          {
            REQUIRE(12);
            if ( (header[4] & 0xf0) == 0x20 )
            {
              // MPEG1
              SYNC;
            }
            else if ( (header[4] & 0xc0) == 0x40 )
            {
              // MPEG2
              REQUIRE(14);
              REQUIRE(14 + size_t(header[13] & 7));
              SYNC;
            }
            else
            {
              // error (false sync?)
              ++errors;
              RESYNC(1);
            }
          }

          case 0xbb: // system header
          case 0xbc: // MPEG2: program stream map
          case 0xbe: // stuffing stream
            DROP;

        } // switch (header[3])

        // reserved streams
        if ( (header[3] & 0xf0) == 0xf0 )
          DROP;

        /////////////////////////////////////////////////////////////////
        // Parse packet flags

        REQUIRE(7);
        size_t pos = 6;
        if ( header[3] != 0xbf ) // Private Stream 2 has no following flags
        {
          if ( (header[pos] & 0xc0) == 0x80 )
          {
            // MPEG2
            REQUIRE(9);
            pos = header[8] + 9;
            REQUIRE(pos);
          }
          else
          {
            // MPEG1
            while ( header[pos] == 0xff && pos < data_size )
            {
              ++pos;
            }

            if ( pos == data_size )
            {
              if ( data_size < 24 )
              {
                REQUIRE(pos+1)
              }
              else
              {
                // too much stuffing (false sync?)
                ++errors;
                RESYNC(1);
              }
            }

            if ( (header[pos] & 0xc0) == 0x40 )
            {
              pos += 2;
              REQUIRE(pos+1);
            }

            if ( (header[pos] & 0xf0) == 0x20 )
              pos += 5;
            else if ( (header[pos] & 0xf0) == 0x30 )
              pos += 10;
            else // if (header[pos] == 0x0f)
              ++pos;

            REQUIRE(pos);
          }
        } // if (header[3] != 0xbf) // Private Stream 2 has no following flags

        /////////////////////////////////////////////////////////////////
        // Substream header

        stream = header[3];
        if ( stream == 0xbd ) // Private Stream 1 has substream header
        {
          ++pos;
          REQUIRE(pos);

          substream = header[pos-1];
          subheader = header + pos;

          // AC3/DTS substream (3 bytes subheader)
          if ( (substream & 0xf0) == 0x80 )
            REQUIRE(pos + 3);
          // LPCM substream (6 bytes subheader)
          if ( (substream & 0xf0) == 0xa0 )
            REQUIRE(pos + 6);
        }
        else
        {
          substream = 0;
          subheader = 0;
        }

        ///////////////////////////////////////////////
        // FINAL
        ///////////////////////////////////////////////

        ++packets;
        header_size = data_size;
        payload_size = 6 + (header[4] << 8) + header[5] - data_size;

        data_size = 0;
        state = state_sync;

        return payload_size;
        // note: never be here
      } // case demux_header:

    } // switch (state)
  } // for ( ; ; )
}

}; // namespace AudioFilter

// vim: ts=2 sts=2 et

