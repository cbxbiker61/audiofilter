#pragma once
#ifndef VALIB_EQUALIZER_MCH_H
#define VALIB_EQUALIZER_MCH_H

#include <math.h>
#include "ConvolverMch.h"
#include "../fir/eq_fir.h"
#include "../fir/multi_fir.h"

namespace AudioFilter {

///////////////////////////////////////////////////////////////////////////////
// EqualizerMch
// Just a wrapper for ConvolverMch and EqFIR
///////////////////////////////////////////////////////////////////////////////

class EqualizerMch : public Filter
{
protected:
  EqFIR master;               // master equalizer
  EqFIR ch_eq[NCHANNELS];     // channel equalizers
  MultiFIR multi_fir[NCHANNELS]; // sum of master and channel equalizers
  const FIRGen *firs[NCHANNELS];

  bool enabled;
  ConvolverMch conv;

public:
  EqualizerMch()
    : enabled(false)
  {
    FIRGen *master_plus_channel[2];
    master_plus_channel[0] = &master;

    for ( int ch_name = 0; ch_name < NCHANNELS; ++ch_name )
    {
      master_plus_channel[1] = &ch_eq[ch_name];
      multi_fir[ch_name].set(master_plus_channel, 2);
      firs[ch_name] = &multi_fir[ch_name];
    }
  }

  ~EqualizerMch()
  {
    conv.release_all_firs();
  }

  /////////////////////////////////////////////////////////
  // Equalizer interface

  bool get_enabled() const { return enabled; }

  void set_enabled(bool new_enabled)
  {
    if (new_enabled != enabled)
    {
      enabled = new_enabled;
      if (enabled)
        conv.set_all_firs(firs);
      else
        conv.release_all_firs();
    }
  }

  // Per-channel equalizers
  // CH_NONE references to master (all-channels) equalizer

  size_t get_nbands(int ch_name) const;
  size_t set_bands(int ch_name, const EqBand *bands, size_t nbands);
  size_t get_bands(int ch_name, EqBand *bands, size_t first_band, size_t nbands) const;
  void reset_eq(int ch_name);

  /////////////////////////////////////////////////////////
  // Filter interface

  virtual void reset()                     { conv.reset();                 }
  virtual bool is_ofdd() const             { return conv.is_ofdd();        }

  virtual bool query_input(Speakers spk) const { return conv.query_input(spk); }
  virtual bool set_input(Speakers spk)     { return conv.set_input(spk);   }
  virtual Speakers get_input() const       { return conv.get_input();      }
  virtual bool process(const Chunk *chunk) { return conv.process(chunk);   }

  virtual Speakers get_output() const      { return conv.get_output();     }
  virtual bool is_empty() const            { return conv.is_empty();       }
  virtual bool get_chunk(Chunk *chunk)     { return conv.get_chunk(chunk); }
};

}; // namespace AudioFilter

#endif

