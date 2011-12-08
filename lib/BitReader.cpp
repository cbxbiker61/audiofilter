#include <AudioFilter/BitReader.h>
#include <cstring>
#ifdef __unix__
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
#include <unistd.h>
#else
#endif

namespace AudioFilter {

void BitReader::init (const uint8_t *buf, size_t bufsize, bool isBigEndian)
{
  if ( _buf )
    delete _buf;

  _buf = new uint8_t[bufsize];
  _bufSize = bufsize;
  _bitN = 0;

  if ( isBigEndian )
  {
    ::memcpy(_buf, buf, bufsize);
  }
  else
  {
#ifdef __unix__
    ::swab(buf, _buf, bufsize);
#else
#error put a byte swapper here
#endif
  }
}

}; // namespace AudioFilter

// vim: ts=2 sts=2 et

