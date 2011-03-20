/*
  Table CRC algorithm speed mainly depends on table access speed.
  If we increase table size (and decrease table accesses) it increases cache
  misses so 8bit table may be considered as optimal choise.

  Some words about 32bit access
  =============================
  This module uses 32bit access everywhere. Why?

  Conversion 16 <-> 32bit word sometimes takes much of time. In some cases it
  is not a problem: 16bit-le with 16bit access works almost at the same speed
  as 32bit access. But 16bit-be works 6 times slower on P4 and 2 times slower
  on P3 because of several 16 <-> 32bit conversions (before and after swab 
  function).

  Also 32bit access is about 30% faster on P4 and about 50% faster on P3 
  compared to simple byte-access CRC algorithm.
*/


#include "crc.h"

const CRC crc16(POLY_CRC16, 16);
const CRC crc32(POLY_CRC32, 32);

///////////////////////////////////////////////////////////////////////////////
// CRC primitives

uint32_t 
CRC::add_bits(uint32_t crc, uint32_t data, size_t bits) const
{
  if (bits)
  {
    crc ^= (data << (32 - bits));
    while (bits--)
      if (crc & 0x80000000)
        crc = (crc << 1) ^ poly;
      else
        crc <<= 1;
  }
  return crc;
}

uint32_t
CRC::add_8(uint32_t crc, uint32_t data) const
{
  return (crc << 8) ^ tbl[(crc >> 24) ^ (data & 0xff)];
}

uint32_t
CRC::add_32(uint32_t crc, uint32_t data) const
{
  crc ^= data;
  crc = (crc << 8) ^ tbl[crc >> 24];
  crc = (crc << 8) ^ tbl[crc >> 24];
  crc = (crc << 8) ^ tbl[crc >> 24];
  crc = (crc << 8) ^ tbl[crc >> 24];
  return crc;
}

///////////////////////////////////////////////////////////////////////////////
// Init CRC table

void
CRC::init(uint32_t _poly, unsigned _power)
{
  unsigned byte;
  assert(_power <= 32);

  poly = _poly << (32 - _power);
  power = _power;

  for (byte = 0; byte < 256; byte++)
    tbl[byte] = add_bits(0, byte, 8);
}

///////////////////////////////////////////////////////////////////////////////
// Calc CRC

uint32_t 
CRC::calc(uint32_t crc, const uint8_t *data, size_t size) const
{
  const uint8_t *end = data + size;

  /////////////////////////////////////////////////////
  // Process unaligned start (8bit)
  // Because data pointer is unaligned we may need up
  // to 3 byte loads to align pointer to 32bit boundary.

  while (data < end && align32(data) != 0)
    crc = add_8(crc, *data++);

  /////////////////////////////////////////////////////
  // Process main block (32bit)

  uint32_t *data32 = (uint32_t *)data;
  uint32_t *end32  = (uint32_t *)(end - align32(end));
  while (data32 < end32)
  {
    crc = add_32(crc, be2uint32(*data32));
    data32++;
  }

  /////////////////////////////////////////////////////
  // Process unaligned end (8bit)
  // We may need up to 3 byte loads to finish the 
  // stream.

  data = (uint8_t *)data32;
  while (data < end)
    crc = add_8(crc, *data++);

  return crc;
}

uint32_t 
CRC::calc_bits(uint32_t crc, const uint8_t *data, size_t start_bit, size_t bits) const
{
  data += start_bit >> 3;
  start_bit &= 7;

  size_t end_bit = start_bit + bits;
  size_t size = end_bit >> 3;
  end_bit &= 7;

  if (size)
  {
    // prolog
    crc = add_bits(crc, *data, 8 - start_bit);
    data++;

    // body
    crc = calc(crc, data, size-1);
    data += size-1;

    // epilog
    crc = add_bits(crc, (*data) >> (8 - end_bit), end_bit);
  }
  else
  {
    // all stream is in one word
    crc = add_bits(crc, (*data) >> (8 - end_bit), bits);
  }

  return crc;
}
