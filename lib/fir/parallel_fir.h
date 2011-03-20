/*
  Combines several FIRs into one in parallel.
  I.e. applies all filters at once and sums the result.
*/

#ifndef VALIB_PARALLEL_FIR_H
#define VALIB_PARALLEL_FIR_H

#include "../fir.h"

class ParallelFIR : public FIRGen
{
protected:
  size_t count;
  const FIRGen **list;
  int sample_rate;

  mutable int ver;
  mutable int list_ver;

public:
  ParallelFIR();
  ParallelFIR(const FIRGen *const *list, size_t count);
  ~ParallelFIR();

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
