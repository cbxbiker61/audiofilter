#pragma once
#ifndef AUDIOFILTER_AUTOFILE_H
#define AUDIOFILTER_AUTOFILE_H

/*
 * AutoFile - Simple file helper class. Supports large files >2G.
 * MemFile - Load the whole file into memory.
 *
 * AutoFile may support large files >2G (currently only in Visual C++). If you
 * open a large file without large file support, file size is set to
 * max_size constant. In this case you can use the file, but cannot seek
 * and get the file position over the max_size limit. Note, that limit is only
 * 2Gb because position in standard library is signed and may not work
 * correctly over this limit.
 *
 * VC6 does not support uint64_t to double cast, therefore we have to use
 * the signed type int64_t.
 *
 * Note, that file size type, fsize_t may be larger than size_t (it's true on
 * 32bit system). It's because the file may be larger than memory address space.
 * Therefore we cannot safely cast fsize_t to size_t.
 *
 * isLarge() helps to detect a large value that cannot be cast to size_t.
 * castSize() casts the value to size_t. Returns -1 when the value is too large.
 */

#ifdef __GNUC__
#include <sys/types.h>
#endif
#include <cstdio>
#include "Defs.h"

namespace AudioFilter {

class AutoFile
{
public:
#ifdef __GNUC__
  typedef off64_t fsize_t;
#else
  typedef int64_t fsize_t;
#endif
  static const fsize_t max_size;

  AutoFile(): f(0)
  {}

  AutoFile(const char *filename, const char *mode = "rb")
    : f(0)
  {
    open(filename, mode);
  }

  AutoFile(FILE *_f, bool _take_ownership = false)
    : f(0)
  {
    open(_f, _take_ownership);
  }

  ~AutoFile()
  {
    close();
  }

  static bool isLarge(fsize_t value)
  {
    return value > (fsize_t)(size_t)-1;
  }

  static size_t castSize(fsize_t value)
  {
    return isLarge(value) ? -1 : (size_t)value;
  }

  bool open(const char *filename, const char *mode = "rb");
  bool open(FILE *_f, bool _take_ownership = false);
  void close(void);

  inline size_t read(void *buf, size_t size)
  {
    return ::fread(buf, 1, size, f);
  }

  inline size_t write(const void *buf, size_t size)
  {
    return ::fwrite(buf, 1, size, f);
  }

  inline bool isOpen(void) const
  {
    return f; // f != NULL
  }

  inline bool eof(void) const
  {
    return f ? (feof(f) != 0) : true;
  }

  inline fsize_t size(void) const
  {
    return filesize;
  }

  inline FILE *fh(void) const
  {
    return f;
  }

  inline operator FILE *(void) const
  {
    return f;
  }

  int seek(fsize_t _pos);
  fsize_t pos(void) const;

protected:
  FILE *f;
  bool own_file;
  fsize_t filesize;

};

class MemFile
{
public:
  MemFile(const char *filename);
  ~MemFile();

  inline size_t size(void) const
  {
    return file_size;
  }

  inline operator uint8_t *(void) const
  {
    return (uint8_t *)data;
  }

protected:
  void *data;
  size_t file_size;

};

}; // namespace AudioFilter

#endif

// vim: ts=2 sts=2 et

