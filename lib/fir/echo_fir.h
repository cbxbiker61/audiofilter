/*
  Echo filter
*/

#ifndef VALIB_ECHO_FIR
#define VALIB_ECHO_FIR

#include "../fir.h"

class EchoFIR : public FIRGen
{
protected:
  int ver;
  vtime_t delay;
  double gain;

public:
  EchoFIR();
  EchoFIR(vtime_t delay, double gain);

  /////////////////////////////////////////////////////////
  // Own interface

  void set(vtime_t delay, double gain);
  void get(vtime_t *delay, double *gain) const;

  void set_delay(vtime_t delay);
  void set_gain(double gain);
  vtime_t get_delay() const;
  double get_gain() const;


  /////////////////////////////////////////////////////////
  // FIRGen interface

  virtual int version() const;
  virtual const FIRInstance *make(int sample_rate) const;
};

#endif
