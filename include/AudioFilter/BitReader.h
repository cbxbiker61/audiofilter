#pragma once
#ifndef AUDIOFILTER_BITREADER_H
#define AUDIOFILTER_BITREADER_H

#include "Defs.h"

namespace AudioFilter {

class BitReader
{
public:
  BitReader()
    : _buf(0)
    , _bufSize(0)
    , _bitN(0)
  {
  }

  BitReader(const BitReader &obj)
    : _buf(0)
    , _bufSize(0)
    , _bitN(0)
  {
    init(obj._buf, obj._bufSize, true);
  }

  BitReader(const uint8_t *buf, size_t bufsize, bool isBigEndian)
    : _buf(0)
    , _bufSize(0)
    , _bitN(0)
  {
    init(buf, bufsize, isBigEndian);
  }

  void initBigEndian (const uint8_t *buf, size_t bufsize)
  {
    init(buf, bufsize, true);
  }

  void initLittleEndian (const uint8_t *buf, size_t bufsize)
  {
    init(buf, bufsize, false);
  }

  void init(const uint8_t *buf, size_t bufsize, bool isBigEndian);

  ~BitReader()
  {
    if ( _buf )
      delete _buf;
  }

  void reset(void)
  {
      _bitN = 0;
  }

  void seek(size_t offset)
  {
      _bitN = offset;
  }

  void skip(size_t bitcount)
  {
      _bitN += bitcount;
  }

  bool getBool(void)
  {
    return getBit();
  }

  uint8_t getByte(size_t bitcount = 8)
  {
    uint8_t ret(0);

    for ( size_t i = 0; i < bitcount; ++i )
    {
      ret <<= 1;
      ret += getBit();
    }

    return ret;
  }

  int getInt(size_t bitcount)
  {
    return (int)getSizeT(bitcount);
  }

  size_t getSizeT(size_t bitcount)
  {
    size_t ret(0);

    for ( size_t i = 0; i < bitcount; ++i )
    {
      ret <<= 1;
      ret |= getBit();
    }

    return ret;
  }

private:
  uint32_t getBit(void)
  {
    size_t bytn(_bitN / 8);
    size_t bitn(7 - (_bitN % 8));
    uint32_t value((_buf[bytn] >> bitn)&1);
    ++_bitN;
    return value;
  }

  uint8_t *_buf;
  size_t _bufSize;
  size_t _bitN;
};

}; // namespace AudioFilter

#endif

// vim: ts=2 sts=2 et

