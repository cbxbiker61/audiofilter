#pragma once
#ifndef AUDIOFILTER_AUTOBUF_H
#define AUDIOFILTER_AUTOBUF_H

/*
 * Simple buffer helper class
 */

#include <cstring>
#include "Defs.h"

namespace AudioFilter {

template <class T> class AutoBuf
{
private:
  T *f_buf;
  size_t f_size;
  size_t f_allocated;

  AutoBuf(const AutoBuf &);
  AutoBuf &operator =(const AutoBuf &);

public:
  AutoBuf(): f_buf(0), f_size(0), f_allocated(0)
  {}

  AutoBuf(size_t size): f_buf(0), f_size(0), f_allocated(0)
  {
    allocate(size);
  }

  ~AutoBuf()
  {
    free();
  }

  inline T *allocate(size_t size)
  {
    if ( f_allocated < size )
    {
      free();
      f_buf = new T[size];

      if (f_buf)
      {
        f_size = size;
        f_allocated = size;
      }
    }
    else
      f_size = size;

    return f_buf;
  }

  inline T *reallocate(size_t size)
  {
    if ( f_allocated < size )
    {
      T *new_buf = new T[size];

      if ( new_buf )
      {
        memcpy(new_buf, f_buf, f_size * sizeof(T));

        if ( f_buf )
        {
          delete[] f_buf;
        }

        f_buf = new_buf;
        f_size = size;
        f_allocated = size;
      }
      else
        free();
    }
    else
      f_size = size;

    return f_buf;
  }

  inline void free(void)
  {
    delete[] f_buf;
	f_buf = 0;
    f_size = 0;
    f_allocated = 0;
  }

  inline void zero(void)
  {
    if ( f_buf )
      memset(f_buf, 0, f_size * sizeof(T));
  }

  inline size_t size(void) const
  {
    return f_size;
  }

  inline size_t allocated(void) const
  {
    return f_allocated;
  }

  inline T *data() const
  {
    return f_buf;
  }

  inline operator T*() const
  {
    return f_buf;
  }

  inline bool isAllocated(void) const
  {
    return f_buf != 0;
  }
};

}; // namespace AudioFilter

#endif

// vim: ts=2 sts=2 et

