#ifndef VALIB_COUNTER_H
#define VALIB_COUNTER_H

#include "../filter.h"


class Counter : public NullFilter
{
protected:
  size_t counter;

  /////////////////////////////////////////////////////////
  // NullFilter overrides

  virtual void on_reset()
  {
    counter = 0;
  }
  virtual bool on_process()
  {
    counter += size;
    return true;
  };

public:
  Counter()
  :NullFilter(-1)
  {
    counter = 0;
  };

  inline size_t get_count() 
  {
    return counter; 
  }
};

#endif
