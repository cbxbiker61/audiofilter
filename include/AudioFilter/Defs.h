#pragma once
#ifndef AUDIOFILTER_DEFS_H
#define AUDIOFILTER_DEFS_H

#include <assert.h>
#include <stddef.h> // size_t

#ifdef __GNUC__
#include <stdint.h>
#else
typedef signed char      int8_t;
typedef signed short     int16_t;
typedef signed int       int32_t;
typedef unsigned char    uint8_t;
typedef unsigned short   uint16_t;
typedef unsigned int     uint32_t;
typedef signed __int64     int64_t;
typedef unsigned __int64   uint64_t;
#endif

#pragma pack(push, 1) // do not justify following structure
struct int24_t // int24_t is a low-endian structure
{
  uint16_t low;
  int8_t   high;

  int24_t() {}

  int24_t(int32_t i)
  {
    low  = uint16_t(i & 0xFFFF);
    high = int8_t(i >> 16);
  }

  int24_t(double d)
  {
    int i = int(d);
    low  = uint16_t(i & 0xFFFF);
    high = int8_t(i >> 16);
  }

  operator int32_t()
  {
    return (high << 16) + low;
  }
};
#pragma pack(pop)

#if defined(_DEBUG)

  // do not use inline functions for debug version
  // it is MUCH faster because in debug version functions are not inlined
# define swab_u32(i) uint32_t((uint32_t(i) >> 24) | (uint32_t(i) >> 8) & 0xff00 | (uint32_t(i) << 8) & 0xff0000 | (uint32_t(i) << 24))
# define swab_s32(i) int32_t(swab_u32(i))
# define swab_u16(i) uint16_t((uint16_t(i) << 8) | (uint16_t(i) >> 8))
# define swab_s16(i) int16_t(swab_u16(i))
  inline int32_t  swab_s24(int24_t i)  { return swab_s32(i) >> 8; }

#elif defined(__GNUC__)
#include <byteswap.h>
#define swab_u32(n) bswap_32(n)
#define swab_s32(n) (int32_t)bswap_32(n)
#define swab_u16(n) bswap_16(n)
#define swab_s16(n) (int16_t)bswap_16(n)

inline int32_t swab_s24(int24_t i)
{
  return swab_s32(i) >> 8;
}

#elif defined(_M_IX86) && defined(__GNUC__)

  inline uint32_t swab_u32(uint32_t x)
  {
    asm("bswap %0" : "=r" (x) : "0" (x));
    return x;
  };

  inline int32_t swab_s32(int32_t x)
  {
    asm("bswap %0" : "=r" (x) : "0" (x));
    return x;
  }

  inline uint16_t swab_u16(uint16_t x)
  {
    asm("bswap %0; shr %0, 16" : "=r" (x) : "0" (x));
    return x;
  }

  inline int16_t swab_s16(int16_t x)
  {
    asm("bswap %0; shr %0, 16" : "=r" (x) : "0" (x));
    return x;
  }

  inline int32_t swab_s24(int24_t x)
  {
    asm("bswap %0; sar %0, 8" : "=r" (x) : "0" (x));
    return x;
  }

#elif defined(_M_IX86)

  // use asm inline functions
# pragma warning(push)
# pragma warning(disable: 4035)
  inline uint32_t swab_u32(uint32_t x)
  {
    __asm mov eax, x
    __asm bswap eax
  }
  inline int32_t swab_s32(int32_t x)
  {
    __asm mov eax, x
    __asm bswap eax
  }
  inline uint16_t swab_u16(uint16_t x)
  {
    __asm mov ax, x
    __asm bswap eax
    __asm shr eax, 16
  }
  inline int16_t swab_s16(int16_t x)
  {
    __asm mov ax, x
    __asm bswap eax
    __asm shr eax, 16
  }
  inline int32_t swab_s24(int24_t x)
  {
    __asm mov eax, x
    __asm bswap eax
    __asm sar eax, 8
  }
# pragma warning(pop)

#else

  // use general inline functions
  //inline uint32_t swab_u32(uint32_t i) { return (i >> 24) | (i >> 8) & 0xff00 | (i << 8) & 0xff0000 | (i << 24); }
  inline uint32_t swab_u32(uint32_t i) { return (i >> 24) | ((i >> 8) & 0xff00) | ((i << 8) & 0xff0000) | (i << 24); }
  inline int32_t  swab_s32(int32_t i)  { return (int32_t)swab_u32((uint32_t)i); }
  inline uint16_t swab_u16(uint16_t i) { return (i << 8) | (i >> 8); }
  inline int16_t  swab_s16(int16_t i)  { return (int16_t)swab_u16((uint16_t)i); }
  inline int32_t  swab_s24(int24_t i)  { return swab_s32(i) >> 8; }
#endif

#if 1
inline float swab_float(float d)
{
    float a;
    uint8_t *dst = (uint8_t *)&a;
    uint8_t *src = (uint8_t *)&d;

    dst[0] = src[3];
    dst[1] = src[2];
    dst[2] = src[1];
    dst[3] = src[0];

    return a;
}
#else
// this function causes dereferencing type-punned pointer warnings
// with gcc
inline float swab_float( float f )
{
  uint32_t i = swab_u32(*(uint32_t *)&f);
  return *(float *)&i;
}
#endif

#ifdef __GNUC__
#include <endian.h> // included with glibc
#endif

#ifndef __LITTLE_ENDIAN
#define __LITTLE_ENDIAN 1234
#endif
#ifndef __BYTE_ORDER
// default to little endian
#define __BYTE_ORDER __LITTLE_ENDIAN
#endif

#if __BYTE_ORDER == __BIG_ENDIAN
#define int2le32(i) swab_s32(i)
#define int2be32(i) (i)
#define int2le24(i) swab_s24(i)
#define int2be24(i) (i)
#define int2le16(i) swab_s16(i)
#define int2be16(i) (i)

#define le2int32(i) swab_s32(i)
#define be2int32(i) (i)
#define le2int24(i) swab_s24(i)
#define be2int24(i) (i)
#define le2int16(i) swab_s16(i)
#define be2int16(i) (i)

#define uint2le32(i) swab_u32(i)
#define uint2be32(i) (i)
#define uint2le24(i) swab_u24(i)
#define uint2be24(i) (i)
#define uint2le16(i) swab_u16(i)
#define uint2be16(i) (i)

#define le2uint32(i) swab_u32(i)
#define be2uint32(i) (i)
#define le2uint24(i) swab_u24(i)
#define be2uint24(i) (i)
#define le2uint16(i) swab_u16(i)
#define be2uint16(i) (i)

#else // ! __BIG_ENDIAN

#define int2be32(i) swab_s32(i)
#define int2le32(i) (i)
#define int2be24(i) swab_s24(i)
#define int2le24(i) (i)
#define int2be16(i) swab_s16(i)
#define int2le16(i) (i)

#define be2int32(i) swab_s32(i)
#define le2int32(i) (i)
#define be2int24(i) swab_s24(i)
#define le2int24(i) (i)
#define be2int16(i) swab_s16(i)
#define le2int16(i) (i)

#define uint2be32(i) swab_u32(i)
#define uint2le32(i) (i)
#define uint2be24(i) swab_u24(i)
#define uint2le24(i) (i)
#define uint2be16(i) swab_u16(i)
#define uint2le16(i) (i)

#define be2uint32(i) swab_u32(i)
#define le2uint32(i) (i)
#define be2uint24(i) swab_u24(i)
#define le2uint24(i) (i)
#define be2uint16(i) swab_u16(i)
#define le2uint16(i) (i)
#endif // ! __BIG_ENDIAN

namespace AudioFilter {

/*
 * Basic data types, compiler-dependent options, constants and utility finctions
 */

const char *valib_build_info();
const char *valib_credits();

///////////////////////////////////////////////////////////////////////////////
//
//                        Compiler-dependent section
//
///////////////////////////////////////////////////////////////////////////////

// __forceinline is MSVC-specific keyword so we have to redefine
// it to usual inline keyword for other compilers
// (define it to always-inline attribute for GCC?)

#if !defined(_MSC_VER) && !defined(__forceinline)
#  define __forceinline inline
#endif

// MSVC8: disable depreciation warning
// MSVC6: disable very long identifier warning

#ifdef _MSC_VER
#  pragma warning(disable: 4996)
#  pragma warning(disable: 4786)
#endif

///////////////////////////////////////////////////////////////////////////////
//
//                              Base constants
//
///////////////////////////////////////////////////////////////////////////////

// one of the most important constants:
// maximum number of channels supported by library

#define NCHANNELS 8

// pi constant

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795029
#endif

// level multipliers

#define LEVEL_PLUS6DB 2.0
#define LEVEL_PLUS3DB 1.4142135623730951
#define LEVEL_3DB     0.7071067811865476
#define LEVEL_45DB    0.5946035575013605
#define LEVEL_6DB     0.5

// bitstream types

#define BITSTREAM_NONE  (-1)
#define BITSTREAM_8     0
#define BITSTREAM_16BE  1
#define BITSTREAM_16LE  2
#define BITSTREAM_32BE  3
#define BITSTREAM_32LE  4
#define BITSTREAM_14BE  5  // DTS bitstream
#define BITSTREAM_14LE  6  // DTS bitstream

///////////////////////////////////////////////////////////////////////////////
//
//                              Type definitions
//
///////////////////////////////////////////////////////////////////////////////

// pos_t - to be used as buffer position and index
// Buffer position should not be int because win64 has 32bit int type and
// 64bit pointers.
typedef ptrdiff_t pos_t;

// integer types

///////////////////////////////////////////////////////////////////////////////
// Valib-specific type definitions:
//
// sample_t - audio sample type
//   All internal audio processing is done with this type. There're 2 sample
//   types supported now: float and double. To use single-precision float type
//   define FLOAT_SAMPLE global symbol.
//
//   SAMPLE_THRESHOLD - minimum difference between samples
//   EQUAL_SAMPLES    - macro to compare two samples
//
//   vtime_t    - time type (see vtime.h)
//   matrix_t   - mixing matrix (see filters\mixer.h)
///////////////////////////////////////////////////////////////////////////////

#ifndef FLOAT_SAMPLE

typedef double sample_t;

#else

typedef float sample_t;

# if _MSC_VER >= 1200
    // most of tables use double-precision constants
    // with float type sample it leads to tons of non-informative warnings
    // warning C4244: '+=' : conversion from 'double' to 'float', possible loss of data
    // warning C4305: 'initializing' : truncation from 'const double' to 'const float'
#   pragma warning (disable: 4244 4305)
# endif

#endif

#define SAMPLE_THRESHOLD (1e-10)
#define EQUAL_SAMPLES(s1, s2) (fabs((s1) - (s2)) < SAMPLE_THRESHOLD)

typedef double vtime_t;

class matrix_t
{
protected:
  sample_t matrix[NCHANNELS][NCHANNELS];

public:
  matrix_t()
  {
    zero();
  }

  matrix_t(const matrix_t &m)
  {
    *this = m;
  }

  inline const sample_t *operator [](int i) const
  {
    assert(i >= 0 && i < NCHANNELS);
    return matrix[i];
  }

  inline sample_t *operator [](int i)
  {
    assert(i >= 0 && i < NCHANNELS);
    return matrix[i];
  }

  bool operator ==(const matrix_t &m) const;
  bool operator !=(const matrix_t &m) const;

  matrix_t &operator =(const matrix_t &m);
  matrix_t &zero();
  matrix_t &identity();
};

///////////////////////////////////////////////////////////////////////////////
// Some utilities
//
//   MIN(a, b)           finds minimum
//   MAX(a, b)           finds maximum
//
//   value2db(value)     convert to decibels (requires math.h)
//   db2value(db)        convert from decibels (requires math.h)
//
//   array_size(array)   calculates array size
//   safe_delete()       deletes a buffer only when it is allocated and
//                       zeroes the pointer
//   align32(ptr)        32bit_aligned_pointer = ptr - align32(ptr);
///////////////////////////////////////////////////////////////////////////////

#ifndef MIN
#  define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#  define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#endif

#define value2db(value) ((value > 0) ? log10(value)*20.0 : 0)
#define db2value(db)    pow(10.0, db/20.0)

#define array_size(array) (sizeof(array) / sizeof(array[0]))
//#define safe_delete(x) { if (x) delete(x); x = 0; }

inline size_t align32(const void *ptr)
{ return (size_t)ptr & 3; }

///////////////////////////////////////////////////////////////////////////////
// Byte-swap functions
//
//   uint32_t swab_u32(uint32_t i)
//   int32_t  swab_s32(int32_t i)
//   uint16_t swab_u16(uint16_t i)
//   int16_t  swab_s16(int16_t i)
//   int32_t  swab_s24(int24_t i)
//
// We use defines for debug version because it works much faster than inline
// functions (overhead of function call is too big). So we must be careful
// with it because following constructions are prohibited:
//
// i = swab_u16(*p++); // WRONG!!! because it is replaced with
//                     // i = ((*p++) << 8) | (*p++) >> 8));
//
// Use asm inlines for X86 architecture and general inlines otherwise.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Byteorder conversion
///////////////////////////////////////////////////////////////////////////////

}; // namespace AudioFilter

#endif

// vim: ts=2 sts=2 et

