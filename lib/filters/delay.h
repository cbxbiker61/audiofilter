/*
  Delay class
  Per-channel delay buffer

  Speakers: unchanged
  Input formats: Linear
  Timing: unchanged
  Buffering: yes
  Parameters:
    units      - units in wich delay values are specified
    delays     - delay values
    [time_shift] - value of time shift should be applied at output:

  todo: time_shift parameter
*/

#ifndef VALIB_DELAY_H
#define VALIB_DELAY_H

#include "../buffer.h"
#include "../filter.h"


#define DELAY_SP 0 // samples
#define DELAY_MS 1 // milliseconds
#define DELAY_M  2 // meters
#define DELAY_CM 3 // centimeters
#define DELAY_FT 4 // feet 
#define DELAY_IN 5 // inches

class Delay : public NullFilter
{
protected:
  bool  enabled;
  int   units;                    // delay units
  float delays[NCHANNELS];        // delay values

  SampleBuf buf;                  // sample buffer
  bool      first_half;           // first/second buffer half is used
  int       ch_delays[NCHANNELS]; // delay values in samples (reordered)
  int       lag;                  // time lag

  double    units2samples(int _units);
  void      init(Speakers _spk);

  /////////////////////////////////////////////////////////
  // NullFilter overrides

  virtual void on_reset();
  virtual bool on_set_input(Speakers _spk);
  virtual bool on_process();

public:
  Delay();

  inline bool get_enabled() const;
  inline void set_enabled(bool _enabled);

  int  get_units() const;
  void set_units(int units);

  void get_delays(float delays[NCHANNELS]) const;
  void set_delays(const float delays[NCHANNELS]);
};


inline bool Delay::get_enabled() const
{ return enabled; }

inline void Delay::set_enabled(bool _enabled)
{ enabled = _enabled; }

#endif
