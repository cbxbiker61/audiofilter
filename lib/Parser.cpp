#include <iostream>
#include <cstdio>
#include <AudioFilter/Parsers.h>

namespace AudioFilter {

///////////////////////////////////////////////////////////////////////////////
// HeaderParser
///////////////////////////////////////////////////////////////////////////////

std::string HeaderParser::getHeaderInfo(const uint8_t *hdr)
{
  char info[1024];
  HeaderInfo h;
  size_t info_size = 0;
  info[0] = 0;

  if ( parseHeader(hdr, &h) )
  {
    info_size += sprintf(info + info_size, "Stream format: %s %s %iHz\n",
			 h.spk.getFormatText(), h.spk.getModeText(), h.spk.sample_rate);

    switch ( h.bs_type )
    {
      case BITSTREAM_8:
        info_size += sprintf(info + info_size, "Bitstream type: byte stream\n");
        break;
      case BITSTREAM_16BE:
        info_size += sprintf(info + info_size, "Bitstream type: 16bit big endian\n");
        break;
      case BITSTREAM_16LE:
        info_size += sprintf(info + info_size, "Bitstream type: 16bit little endian\n");
        break;
      case BITSTREAM_14BE:
        info_size += sprintf(info + info_size, "Bitstream type: 14bit big endian\n");
        break;
      case BITSTREAM_14LE:
        info_size += sprintf(info + info_size, "Bitstream type: 14bit little endian\n");
        break;
      default:
        info_size += sprintf(info + info_size, "Bitstream type: unknown\n");
        break;
    }

    if ( h.frame_size )
      info_size += sprintf(info + info_size, "Frame size: %zu\n", h.frame_size);
    else
      info_size += sprintf(info + info_size, "Frame size: free format\n");

    info_size += sprintf(info + info_size, "Samples: %zu\n", h.nsamples);

    if ( h.frame_size > 0 && h.nsamples > 0 )
      info_size += sprintf(info + info_size, "Bitrate: %zukbps\n"
                    , h.frame_size * h.spk.sample_rate * 8 / h.nsamples / 1000);
    else
      info_size += sprintf(info + info_size, "Bitrate: unknown\n");

    if ( h.spdif_type )
      info_size += sprintf(info + info_size, "SPDIF stream type: 0x%x\n", h.spdif_type);
  }
  else
  {
    info_size += sprintf(info + info_size, "No header found\n");
  }

  return info;
}

///////////////////////////////////////////////////////////////////////////////
// StreamBuffer
///////////////////////////////////////////////////////////////////////////////

StreamBuffer::StreamBuffer()
  : _parser(0)
  , _headerSize(0)
  , _minFrameSize(0)
  , _maxFrameSize(0)
  , _buf()
  , _headerBuf(new uint8_t[65536])
  , _hinfo()
  , _sync()
  , _preFrameSize(0)
  , _debris()
  , _frame()
  , _isInSync(false)
  , _isNewStream(false)
{
  _hinfo.drop();
}

StreamBuffer::StreamBuffer(HeaderParser *parser)
  : _parser(0)
  , _headerSize(0)
  , _minFrameSize(0)
  , _maxFrameSize(0)
  , _buf()
  , _headerBuf(new uint8_t[65536])
  , _hinfo()
  , _sync()
  , _preFrameSize(0)
  , _debris()
  , _frame()
  , _isInSync(false)
  , _isNewStream(false)
{
  _hinfo.drop();
  setParser(parser);
}

StreamBuffer::~StreamBuffer()
{
  delete[] _headerBuf;
}

bool StreamBuffer::setParser(HeaderParser *parser)
{
  releaseParser();

  if ( parser
        && _buf.allocate(parser->getMaxFrameSize() * 3 + parser->getHeaderSize() * 2) )
  {
    _parser = parser;
    _headerSize = _parser->getHeaderSize();
    _minFrameSize = _parser->getMinFrameSize();
    _maxFrameSize = _parser->getMaxFrameSize();
    //_headerBuf = _buf.data();
    _sync.ptr = _buf.data() + _headerSize;
    _sync.size = _maxFrameSize * 3 + _headerSize;

    return true;
  }

  return false;
}

void StreamBuffer::releaseParser(void)
{
  reset();

  _parser = 0;
  _headerSize = 0;
  _minFrameSize = 0;
  _maxFrameSize = 0;

  //_headerBuf = 0;
  _sync.ptr = 0;
  _sync.size = 0;
}

void StreamBuffer::reset(void)
{
  _hinfo.drop();
  _sync.dataSize = 0;
  _preFrameSize = _maxFrameSize;

  _debris.ptr = 0;
  _debris.size = 0;

  _frame.ptr = 0;
  _frame.size = 0;
  _frame.interval = 0;

  _isInSync = false;
  _isNewStream = false;
}

bool StreamBuffer::loadBuffer(uint8_t **data, uint8_t *end, size_t required_size)
{
  if ( required_size <= _sync.dataSize )
    return true;

  if ( (_sync.dataSize + (end - *data)) < required_size )
  {
    const size_t loadSize(end - *data);

    ::memcpy(_sync.ptr + _sync.dataSize, *data, loadSize);
    _sync.dataSize += loadSize;
    *data += loadSize;
    return false;
  }
  else
  {
    const size_t loadSize(required_size - _sync.dataSize);
    ::memcpy(_sync.ptr + _sync.dataSize, *data, loadSize);
    _sync.dataSize += loadSize;
    *data += loadSize;
    return true;
  }
}

bool StreamBuffer::loadFrame(uint8_t **data, uint8_t *end)
{
  while ( *data < end || isFrameLoaded() || isDebrisExists() )
  {
    load(data, end);

    if ( isFrameLoaded() )
      return true;
  }

  return false;
}

bool StreamBuffer::load(uint8_t **data, uint8_t *end)
{
  if ( ! _parser )
  {
    *data = end; // avoid endless loop
    return false;
  }

  if ( ! _isInSync )
    return reSync(data, end);

  if ( isFrameLoaded() ) // Drop old debris and frame data
  {
    dropBuffer(_debris.size + _frame.size);
    _debris.size = 0;
    _frame.size = 0;
  }

  _isNewStream = false;

  /////////////////////////////////////////////////////////////////////////////
  // Load next frame

  _frame.ptr = _sync.ptr;
  uint8_t *frame_max(_sync.ptr);

  /* not sure, but with interleaved frames this might
   * not be the best algorithm, probably better to just add hinfo.scan_size
   */
  if ( _hinfo.frame_size && _hinfo.frame_size < _hinfo.scan_size )
    frame_max += _hinfo.scan_size - _hinfo.frame_size;

  while ( _frame.ptr <= frame_max )
  {
    if ( ! loadBuffer(data, end, _frame.ptr - _sync.ptr + _headerSize) )
      return false;

    HeaderInfo hi;

    if ( ! _parser->parseHeader(_frame.ptr, &hi) )
    {
      ++_frame.ptr;
      continue;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Load the rest of the frame

    if ( ! loadBuffer(data, end, _frame.ptr - _sync.ptr
            + (hi.frame_size ? hi.frame_size : _frame.interval)) )
      return false;

    ///////////////////////////////////////////////////////////////////////////
    // DONE! Prepare new frame output.

    if ( _hinfo.frame_size )
      _frame.interval = _hinfo.frame_size + _frame.ptr - _sync.ptr;

    ::memcpy(_headerBuf, _frame.ptr, _hinfo.frame_size);
    _parser->parseHeader(_headerBuf, &_hinfo);

    _debris.ptr = _sync.ptr;
    _debris.size = _frame.ptr - _sync.ptr;

    _frame.size = ( _hinfo.frame_size ) ? _hinfo.frame_size : _frame.interval;

    ++_frame.count;
    return true;
  }

  /////////////////////////////////////////////////////////////////////////////
  // No correct syncpoint found. Resync.

  _isInSync = false;
  _preFrameSize = _maxFrameSize;

  _frame.ptr = 0;
  _frame.size = 0;
  _frame.interval = 0;
  return reSync(data, end);
}

///////////////////////////////////////////////////////////////////////////////
// When we sync on a new stream we may start syncing in between of two
// syncpoints. The data up to the first syncpoint may be wrongly interpreted
// as PCM data and poping sound may occur before the first frame. To prevent
// this we must consider this part as debris before the first syncpoint.
//
// But amount of this data may not be larger than the maximum frame size:
//
// Case 1: The data before the first frame is a part of the stream
// |--------------+------------------------+---
// | < frame_size |          frame1        |  ....
// |--------------+------------------------+---
//
// Case 2: The data before the first frame is not a part of the stream
// |----------------------------+------------------------+---
// |       > frame_size         |          frame1        |  ....
// |----------------------------+------------------------+---
//
// Therefore if we have not found the syncpoint at first frame_size bytes,
// all this data does not belong to a stream.

bool StreamBuffer::reSync(uint8_t **data, uint8_t *end)
{
  assert(*data <= end);
  assert(!_isInSync && !_isNewStream);
  assert(_frame.ptr == 0 && _frame.size == 0 && _frame.interval == 0);

  /////////////////////////////////////////////////////////////////////////////
  // Drop debris

  dropBuffer(_debris.size);
  _debris.ptr = 0;
  _debris.size = 0;

  /////////////////////////////////////////////////////////////////////////////
  // Cache data

  if ( _sync.dataSize < _sync.size )
  {
    const size_t loadSize(MIN(size_t(end - *data), (_sync.size - _sync.dataSize)));
    ::memcpy(_sync.ptr + _sync.dataSize, *data, loadSize);
    _sync.dataSize += loadSize;
    *data += loadSize;
  }

  /////////////////////////////////////////////////////////////////////////////
  // Search 1st syncpoint

  uint8_t *pf1(_sync.ptr);
  uint8_t *pf1Max(_sync.ptr + _preFrameSize);

  while ( pf1 <= pf1Max )
  {
    if ( ! loadBuffer(data, end, pf1 - _sync.ptr + _headerSize) )
      return false;

    HeaderInfo hi1;

    if ( _parser->parseHeader(pf1, &hi1) )
    {
      ///////////////////////////////////////////////////////////////////////////
      // Search 2nd syncpoint
      // * Known frame size: search next syncpoint after the end of the frame but
      //   not further than scan size (if defined)
      // * Unknown frame size: search next syncpoint after the minimum frame size
      //   up to scan size or maximum frame size if scan size is unspecified.

      uint8_t *pf2;
      uint8_t *pf2Max;

      if ( hi1.frame_size )
      {
        std::cout << "FrameSize1=" << hi1.frame_size << std::endl;
        pf2 = pf1 + hi1.frame_size;
        pf2Max = pf1 + MAX(hi1.scan_size, hi1.frame_size);
      }
      else
      {
        pf2 = pf1 + _minFrameSize;
        pf2Max = pf1 + (hi1.scan_size ? hi1.scan_size : _maxFrameSize);
      }

      while ( pf2 <= pf2Max )
      {
        if ( ! loadBuffer(data, end, pf2 - _sync.ptr + _headerSize) )
          return false;

        HeaderInfo hi2;

        if ( ! _parser->parseHeader(pf2, &hi2) )
        {
          ++pf2;
          continue;
        }

        /////////////////////////////////////////////////////////////////////////
        // Search 3rd syncpoint
        // * Known frame size: search next syncpoint after the end of the frame
        //   but not further than scan size (if defined)
        // * Unknown frame size: expect next syncpoint exactly after frame
        //   interval.

        uint8_t *pf3;
        uint8_t *pf3Max;

        if ( hi2.frame_size )
        {
          std::cout << "FrameSize2=" << hi2.frame_size << std::endl;
          pf3 = pf2 + hi2.frame_size;
          pf3Max = pf2 + MAX(hi2.scan_size, hi2.frame_size);
        }
        else
        {
          pf3 = pf2 + (pf2 - pf1);
          pf3Max = pf2 + (pf2 - pf1);
        }

        while ( pf3 <= pf3Max )
        {
          if ( ! loadBuffer(data, end, pf3 - _sync.ptr + _headerSize) )
            return false;

          if ( ! _parser->parseHeader(pf3) )
          {
            ++pf3;
            continue;
          }

          ///////////////////////////////////////////////////////////////////////
          // Does the data before the first frame belong to the stream?
          // If not, send it separately from the first frame

          _debris.ptr = _sync.ptr;
          _debris.size = pf1 - _sync.ptr;

          //const size_t tmpFrameSize(hinfo.frame_size ? hinfo.frame_size : pf2 - pf1);
          const size_t tmpFrameSize(hi1.frame_size ? hi1.frame_size : pf2 - pf1);

          if ( _debris.size > tmpFrameSize )
            return true;

          ///////////////////////////////////////////////////////////////////////
          // DONE! Prepare first frame output.

          //::memcpy(_headerBuf, pf1, _headerSize);
          ::memcpy(_headerBuf, pf1, hi1.frame_size);
          _parser->parseHeader(_headerBuf, &_hinfo);

          _frame.ptr = pf1;
          _frame.interval = pf2 - pf1;
          _frame.size = _hinfo.frame_size ? _hinfo.frame_size : _frame.interval;

          _isInSync = true;
          _isNewStream = true;

          ++_frame.count;
          return true;
        }

        /////////////////////////////////////////////////////////////////////////
        // No correct 3rd syncpoint found.
        // Continue 2nd syncpoint scanning.

        ++pf2;

      } // while (pf2 <= pf2Max)

      /////////////////////////////////////////////////////////////////////////
      // No correct sync sequence found.
      /////////////////////////////////////////////////////////////////////////

    } // if (_parser->parse_header(pf1, &hdr))

    ++pf1;

  } // while (pf1 <= pf1Max)

  /////////////////////////////////////////////////////////////////////////////
  // Return debris
  // * Sync buffer does not start with a syncpoint
  // * No correct sync sequence found (false sync)
  //
  // Try to locate next syncpoint and return data up to the position found
  // as debris.

  uint8_t *pos(pf1 + 1);
  uint8_t *posMax(_sync.ptr + _sync.dataSize - _headerSize);

  while ( pos <= posMax )
  {
    if ( _parser->parseHeader(pos) )
      break;

    ++pos;
  }

  _preFrameSize = 0;
  _debris.ptr = _sync.ptr;
  _debris.size = pos - _sync.ptr;
  return true;
}

bool StreamBuffer::flush(void)
{
  dropBuffer(_debris.size + _frame.size);
  _debris.size = 0;
  _frame.size = 0;

  _isInSync = false;
  _isNewStream = false;

  _debris.ptr = _sync.ptr;
  _debris.size = _sync.dataSize;

  return _debris.size > 0;
}

void StreamBuffer::dropBuffer(size_t size)
{
  assert(_sync.dataSize >= 0);

  if ( ! _sync.dataSize )
    return;

  if ( _sync.dataSize < size )
  {
    std::cout << "Messed up in dropBuf _sync.dataSize=" << _sync.dataSize
            << " size=" << size << std::endl;
  }
  assert(_sync.dataSize >= size);
  _sync.dataSize -= size;
  ::memmove(_sync.ptr, _sync.ptr + size, _sync.dataSize);
}

std::string StreamBuffer::getStreamInfo(void) const
{
  char info[1024];
  size_t info_size(0);
  info[0] = 0; // make sure string is initialized

  if ( _parser )
  {
    if ( _isInSync )
    {
      const std::string &hi(_parser->getHeaderInfo(_frame.ptr));
      info_size += hi.size();
      ::strcpy(info, hi.c_str());
      info_size += sprintf(info + info_size, "Frame interval: %zu\n"
                      , _frame.interval);

      if ( _frame.interval > 0 && _hinfo.nsamples > 0 )
      {
        info_size += sprintf(info + info_size, "Actual bitrate: %zukbps\n"
            , _frame.interval * _hinfo.spk.sample_rate * 8 / _hinfo.nsamples / 1000);
      }
    }
    else
      info_size += sprintf(info + info_size, "Out of sync");
  }
  else
    info_size += sprintf(info + info_size, "No parser set");

  return info;
}

}; // namespace AudioFilter

// vim: ts=2 sts=2 et

