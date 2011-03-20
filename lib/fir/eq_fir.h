/*
  Graphic equalizer
*/

#ifndef VALIB_EQ_FIR_H
#define VALIB_EQ_FIR_H

#include "../fir.h"
#include "../auto_buf.h"

struct EqBand
{
  int freq;
  double gain;
};

class EqFIR : public FIRGen
{
protected:
  int ver; // response version

  // bands info
  size_t nbands;
  AutoBuf<EqBand> bands;
  double ripple;

public:
  EqFIR();
  EqFIR(const EqBand *bands, size_t nbands);

  /////////////////////////////////////////////////////////
  // Equalizer interface

  size_t get_nbands() const;
  size_t set_bands(const EqBand *bands, size_t nbands);
  size_t get_bands(EqBand *bands, size_t first_band, size_t nbands) const;
  double get_ripple() const;
  void set_ripple(double ripple_db);
  void reset();

  /////////////////////////////////////////////////////////
  // FIRGen interface

  virtual int version() const;
  virtual const FIRInstance *make(int sample_rate) const;
};

#endif
