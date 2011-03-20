#include "slice.h"

SliceFilter::SliceFilter(size_t _start, size_t _end):
NullFilter(-1), pos(0), start(_start), end(_end) 
{
  assert(start <= end);
}

void
SliceFilter::init(size_t _start, size_t _end)
{
  reset();
  start = _start;
  end = _end;
  assert(start <= end);
}

void
SliceFilter::on_reset()
{
  pos = 0;
}

bool
SliceFilter::on_process()
{
  // ignore everything after the end (except eos)

  if (pos >= end)
  {
    size = 0;
    return true;
  }

  // ignore everything before the beginning (except eos)
  if (pos + size <= start)
  {
    pos += size;
    size = 0;
    return true;
  }

  // cut off the tail
  if (pos + size > end)
    size = end - pos;

  // cut of the head
  if (pos < start)
  {
    if (spk.is_linear())
      drop_samples(start - pos);
    else
      drop_rawdata(start - pos);
    pos = start;
  }

  pos += size;
  return true;
}
