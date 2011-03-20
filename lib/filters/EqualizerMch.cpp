#include "EqualizerMch.h"

namespace AudioFilter {

size_t EqualizerMch::get_nbands(int ch_name) const
{
  if (ch_name == CH_NONE) return master.get_nbands();
  if (ch_name < 0 || ch_name > NCHANNELS) return 0;
  return ch_eq[ch_name].get_nbands();
}

size_t EqualizerMch::set_bands(int ch_name, const EqBand *bands, size_t nbands)
{
  if (ch_name == CH_NONE) return master.set_bands(bands, nbands);
  if (ch_name < 0 || ch_name > NCHANNELS) return 0;
  return ch_eq[ch_name].set_bands(bands, nbands);
}

size_t EqualizerMch::get_bands(int ch_name, EqBand *bands, size_t first_band, size_t nbands) const
{
  if (ch_name == CH_NONE) return master.get_bands(bands, first_band, nbands);
  if (ch_name < 0 || ch_name > NCHANNELS) return 0;
  return ch_eq[ch_name].get_bands(bands, first_band, nbands);
}

void EqualizerMch::reset_eq(int ch_name)
{
  if (ch_name == CH_NONE) { master.reset(); return; }
  if (ch_name < 0 || ch_name > NCHANNELS) return;
  ch_eq[ch_name].reset();
}

}; // namespace AudioFilter

