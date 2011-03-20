/*
  Simple delay filter (only integer number of samples delay)
*/

#ifndef VALIB_DELAY_FIR
#define VALIB_DELAY_FIR

#include "../fir.h"

class DelayFIR : public FIRGen
{
protected:
  int ver;
  vtime_t delay;

public:
  DelayFIR();
  DelayFIR(vtime_t delay);

  /////////////////////////////////////////////////////////
  // Own interface

  void set_delay(vtime_t delay);
  vtime_t get_delay() const;

  /////////////////////////////////////////////////////////
  // FIRGen interface

  virtual int version() const;
  virtual const FIRInstance *make(int sample_rate) const;
};

#endif
