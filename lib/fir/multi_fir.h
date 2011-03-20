/*
  Combines several FIRs into one sequentially.
  I.e. applies all filters one by one.
*/

#ifndef VALIB_MULTI_FIR_H
#define VALIB_MULTI_FIR_H

#include "../fir.h"

class MultiFIR : public FIRGen
{
protected:
  size_t count;
  const FIRGen **list;
  int sample_rate;

  mutable int ver;
  mutable int list_ver;

public:
  MultiFIR();
  MultiFIR(const FIRGen *const *list, size_t count);
  ~MultiFIR();

  /////////////////////////////////////////////////////////
  // Own interface

  void set(const FIRGen *const *list, size_t count);
  void release();

  /////////////////////////////////////////////////////////
  // FIRGen interface

  virtual int version() const;
  virtual const FIRInstance *make(int sample_rate) const;

};

#endif
