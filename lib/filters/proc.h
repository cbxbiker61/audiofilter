#pragma once
#ifndef AUDIOFILTER_PROC_H
#define AUDIOFILTER_PROC_H

/*
  Audio processor filter
  Acts as high-level manager class.

  Functions: input/output format conversions, input/output levels,
             matrix mixer, agc, drc, delay, dejitter

  Speakers: can change format and mask
  Input formats: PCMxx, Linear
  Output formats: PCMxx, Linear
  Format conversions:
    PCMxx  -> PCMxx
    PCMxx  -> Linear
    Linear -> PCMxx
    Linear -> Linear
  Paramteers:

  // Channel order
  input_order         - input channel order (for PCM only)
  output_order        - output channel order (for PCM only)

  // AGC
  auto_gain           - apply auto gain control
  normalize           - one-pass normalization
  attack              - attack speed (dB/s)
  release             - release speed (dB/s)

  // DRC
  drc                 - apply DRC control
  drc_power           - DRC power (gain in dB at -50dB level)
  drc_level           - current DRC gain (read-only)

  // SRC
  src_quality         - Passband width (0..1)
  src_att             - Stopband attenuation (dB)

  // Bass redirection
  bass_redir          - apply bass redirection
  bass_freq           - bass redirection frequency

  // Matrix & options
  matrix              - mixing matrix
  auto_matrix         - update matrix automatically
  normalize_matrix    - normalize matrix so output gain <= 1.0 (matrix parameter)
  voice_control       - allow center gain change even if it is no center channel at input (matrix parameter)
  expand_stereo       - allow surround gain change even if it is no surround channels at input (matrix parameter)
  clev                - center mix level (matrix parameter)
  slev                - surround mix level (matrix parameter)
  lfelev              - LFE mix level (matrix parameter)

  // Gains
  master              - master gain
  gain                - current gain (read-only)
  input_gains         - input channel's gains
  output_gains        - output channel's gains

  // Delay
  delay               - apply delay to output channels
  delay_units         - delay units
  delays              - delay values

  // Input/output levels
  input_levels        - input levels (read-only)
  output_levels       - output levels (read-only)

  // levels histogram
  dbpb                - dB per bin in histogram
  histogram           - levels histogram (read-only)
  max_level           - maximum level (read-only)

  todo: use state machine instead of filter chain?
*/

#include "../filter_graph.h"
#include "cache.h"
#include "EqualizerMch.h"
#include "levels.h"
#include "mixer.h"
#include "resample.h"
#include "bass_redir.h"
#include "agc.h"
#include "delay.h"
#include "dither.h"
#include "convert.h"
#include "proc_state.h"

namespace AudioFilter {

class AudioProcessor : public Filter
{
protected:
  Speakers in_spk;   // actual input format
  Speakers user_spk; // user-specified format (may be partially-specified)
  Speakers out_spk;  // actual output format

  // dithering

  int dithering;
  double dithering_level() const;

  // filters
  Converter    in_conv;
  CacheFilter  in_cache;
  Levels       in_levels;

  Mixer        mixer;
  Resample     resample;
  EqualizerMch equalizer;
  Dither       dither;
  BassRedir    bass_redir;
  AGC          agc;
  Delay        delay;

  Levels       out_levels;
  CacheFilter  out_cache;
  Converter    out_conv;

  FilterChain chain;
  bool rebuild_chain();

public:
  AudioProcessor(size_t nsamples);

  /////////////////////////////////////////////////////////
  // AudioProcessior interface

  size_t get_info(char *buf, size_t len) const;
  bool query_user(Speakers user_spk) const;
  bool set_user(Speakers user_spk);
  Speakers get_user() const;

  Speakers user2output(Speakers in_spk, Speakers user_spk) const;

  /////////////////////////////////////////////////////////
  // Filter interface

  virtual void reset();
  virtual bool is_ofdd() const;

  virtual bool query_input(Speakers spk) const;
  virtual bool set_input(Speakers spk);
  virtual Speakers get_input() const;
  virtual bool process(const Chunk *chunk);

  virtual Speakers get_output() const;
  virtual bool is_empty() const;
  virtual bool get_chunk(Chunk *chunk);

  /////////////////////////////////////////////////////////
  // Options

  // Channel order

  inline void     get_input_order(int order[NCHANNELS]) const;
  inline void     get_output_order(int order[NCHANNELS]) const;

  inline void     set_input_order(const int order[NCHANNELS]);
  inline void     set_output_order(const int order[NCHANNELS]);

  // Master gain

  inline sample_t get_master() const;
  inline sample_t get_gain() const; // r/o
  inline void     set_master(sample_t gain);

  // AGC

  inline bool     get_auto_gain() const;
  inline bool     get_normalize() const;
  inline sample_t get_attack() const;
  inline sample_t get_release() const;

  inline void     set_auto_gain(bool auto_gain);
  inline void     set_normalize(bool normalize);
  inline void     set_attack(sample_t attack);
  inline void     set_release(sample_t release);

  // DRC

  inline bool     get_drc() const;
  inline sample_t get_drc_power() const;
  inline sample_t get_drc_level() const; // r/o

  inline void     set_drc(bool drc);
  inline void     set_drc_power(sample_t drc_power);

  // Matrix

  inline void     get_matrix(matrix_t &matrix) const;
  inline void     set_matrix(const matrix_t &matrix);

  // Automatrix options

  inline bool     get_auto_matrix() const;
  inline bool     get_normalize_matrix() const;
  inline bool     get_voice_control() const;
  inline bool     get_expand_stereo() const;

  inline void     set_auto_matrix(bool auto_matrix);
  inline void     set_normalize_matrix(bool normalize_matrix);
  inline void     set_voice_control(bool voice_control);
  inline void     set_expand_stereo(bool expand_stereo);

  // Automatrix levels

  inline sample_t get_clev() const;
  inline sample_t get_slev() const;
  inline sample_t get_lfelev() const;

  inline void     set_clev(sample_t clev);
  inline void     set_slev(sample_t slev);
  inline void     set_lfelev(sample_t lfelev);

  // Input/output gains

  inline void     get_input_gains(sample_t input_gains[NCHANNELS]) const;
  inline void     get_output_gains(sample_t output_gains[NCHANNELS]) const;

  inline void     set_input_gains(const sample_t input_gains[NCHANNELS]);
  inline void     set_output_gains(const sample_t output_gains[NCHANNELS]);

  // Input/output levels

  inline void     get_input_levels(vtime_t time, sample_t input_levels[NCHANNELS]); // r/o
  inline void     get_output_levels(vtime_t time, sample_t output_levels[NCHANNELS]); // r/o

  // SRC

  inline double   get_src_quality() const;
  inline double   get_src_att() const;

  inline void     set_src_quality(double q);
  inline void     set_src_att(double a);

  // Eqalizer

  inline bool     get_eq() const;
  inline void     set_eq(bool eq);

  inline size_t   get_eq_nbands(int ch) const;
  inline size_t   get_eq_bands(int ch, EqBand *bands, size_t first_band, size_t nbands) const;
  inline size_t   set_eq_bands(int ch, const EqBand *bands, size_t nbands);

  // Bass redirection

  inline bool     get_bass_redir() const;
  inline int      get_bass_freq() const;

  inline void     set_bass_redir(bool bass_redir);
  inline void     set_bass_freq(int freq);

  // Delays

  inline bool     get_delay() const;
  inline void     set_delay(bool delay);

  inline int      get_delay_units() const;
  inline void     get_delays(float delays[NCHANNELS]) const;

  inline void     set_delay_units(int delay_units);
  inline void     set_delays(const float delays[NCHANNELS]);

  // Dithering

  inline int      get_dithering() const;
  inline void     set_dithering(int dithering);

  // Input/output cache

  inline vtime_t  get_input_cache_size() const;
  inline vtime_t  get_output_cache_size() const;
  inline void     set_input_cache_size(vtime_t size);
  inline void     set_output_cache_size(vtime_t size);

  inline vtime_t  get_input_cache_time() const;
  inline vtime_t  get_output_cache_time() const;
  inline size_t   get_input_cache(int ch_name, vtime_t time, sample_t *buf, size_t size);
  inline size_t   get_output_cache(int ch_name, vtime_t time, sample_t *buf, size_t size);

  // Input/output histogram

  inline int      get_dbpb() const;
  inline void     set_dbpb(int dbpb);

  inline void     get_input_histogram(double *input_histogram, size_t count) const; // r/o
  inline void     get_input_histogram(int ch, double *input_histogram, size_t count) const; // r/o
  inline void     get_output_histogram(double *output_histogram, size_t count) const; // r/o
  inline void     get_output_histogram(int ch, double *output_histogram, size_t count) const; // r/o
  inline sample_t get_max_level() const;
  inline sample_t get_max_level(int ch) const;

  // State

  AudioProcessorState *get_state(vtime_t time);
  void set_state(const AudioProcessorState *state);
};


///////////////////////////////////////////////////////////////////////////////
// AudioProcessor inlines
///////////////////////////////////////////////////////////////////////////////

// Channel order

inline void AudioProcessor::get_input_order(int _order[NCHANNELS]) const
{ in_conv.get_order(_order); }

inline void AudioProcessor::get_output_order(int _order[NCHANNELS]) const
{ out_conv.get_order(_order); }

inline void AudioProcessor::set_input_order (const int _order[NCHANNELS])
{ in_conv.set_order(_order); }

inline void AudioProcessor::set_output_order(const int _order[NCHANNELS])
{ out_conv.set_order(_order); }

// Master gain

inline sample_t AudioProcessor::get_master() const
{ return mixer.get_gain(); }

inline sample_t AudioProcessor::get_gain() const
{ return mixer.get_gain() * agc.gain; }

inline void AudioProcessor::set_master(sample_t _gain)
{ mixer.set_gain(_gain); agc.gain = 1.0; }

// AGC

inline bool AudioProcessor::get_auto_gain() const
{ return agc.auto_gain; }

inline bool AudioProcessor::get_normalize() const
{ return agc.normalize; }

inline sample_t AudioProcessor::get_attack() const
{ return agc.attack; }

inline sample_t AudioProcessor::get_release() const
{ return agc.release; }

inline void AudioProcessor::set_auto_gain(bool _auto_gain)
{ agc.auto_gain = _auto_gain; }

inline void AudioProcessor::set_normalize(bool _normalize)
{ agc.normalize = _normalize; }

inline void AudioProcessor::set_attack(sample_t _attack)
{ agc.attack = _attack; }

inline void AudioProcessor::set_release(sample_t _release)
{ agc.release = _release; }

// DRC

inline bool AudioProcessor::get_drc() const
{ return agc.drc; }

inline sample_t AudioProcessor::get_drc_power() const
{ return agc.drc_power; }

inline sample_t AudioProcessor::get_drc_level() const
{ return agc.drc_level; }

inline void AudioProcessor::set_drc(bool _drc)
{ agc.drc = _drc; }

inline void AudioProcessor::set_drc_power(sample_t _drc_power)
{ agc.drc_power = _drc_power; }

// Matrix

inline void AudioProcessor::get_matrix(matrix_t &_matrix) const
{ mixer.get_matrix(_matrix); }

inline void AudioProcessor::set_matrix(const matrix_t &_matrix)
{ mixer.set_matrix(_matrix); }

// Automatrix options

inline bool AudioProcessor::get_auto_matrix() const
{ return mixer.get_auto_matrix(); }

inline bool AudioProcessor::get_normalize_matrix() const
{ return mixer.get_normalize_matrix(); }

inline bool AudioProcessor::get_voice_control() const
{ return mixer.get_voice_control(); }

inline bool AudioProcessor::get_expand_stereo() const
{ return mixer.get_expand_stereo(); }

inline void AudioProcessor::set_auto_matrix(bool _auto_matrix)
{ mixer.set_auto_matrix(_auto_matrix); }

inline void AudioProcessor::set_normalize_matrix(bool _normalize_matrix)
{ mixer.set_normalize_matrix(_normalize_matrix); }

inline void AudioProcessor::set_voice_control(bool _voice_control)
{ mixer.set_voice_control(_voice_control); }

inline void AudioProcessor::set_expand_stereo(bool _expand_stereo)
{ mixer.set_expand_stereo(_expand_stereo); }

// Automatrix levels

inline sample_t AudioProcessor::get_clev() const
{ return mixer.get_clev(); }

inline sample_t AudioProcessor::get_slev() const
{ return mixer.get_slev(); }

inline sample_t AudioProcessor::get_lfelev() const
{ return mixer.get_lfelev(); }

inline void AudioProcessor::set_clev(sample_t _clev)
{ mixer.set_clev(_clev); }

inline void AudioProcessor::set_slev(sample_t _slev)
{ mixer.set_slev(_slev); }

inline void AudioProcessor::set_lfelev(sample_t _lfelev)
{ mixer.set_lfelev(_lfelev); }

// Input/output gains

inline void AudioProcessor::get_input_gains(sample_t _input_gains[NCHANNELS]) const
{ mixer.get_input_gains(_input_gains); }

inline void AudioProcessor::get_output_gains(sample_t _output_gains[NCHANNELS]) const
{ mixer.get_output_gains(_output_gains); }

inline void AudioProcessor::set_input_gains(const sample_t _input_gains[NCHANNELS])
{ mixer.set_input_gains(_input_gains); }

inline void AudioProcessor::set_output_gains(const sample_t _output_gains[NCHANNELS])
{ mixer.set_output_gains(_output_gains); }

// Input/output levels

inline void AudioProcessor::get_input_levels(vtime_t _time, sample_t _input_levels[NCHANNELS])
{ in_levels.get_levels(_time, _input_levels); };

inline void AudioProcessor::get_output_levels(vtime_t _time, sample_t _output_levels[NCHANNELS])
{ out_levels.get_levels(_time, _output_levels); }

// SRC

inline double AudioProcessor::get_src_quality() const
{ return resample.get_quality(); }

inline double AudioProcessor::get_src_att() const
{ return resample.get_attenuation(); }

inline void AudioProcessor::set_src_quality(double q)
{ resample.set_quality(q); }

inline void AudioProcessor::set_src_att(double a)
{ resample.set_attenuation(a); }

// Equalizer

inline bool AudioProcessor::get_eq() const
{ return equalizer.get_enabled(); }

inline void AudioProcessor::set_eq(bool eq)
{
  equalizer.set_enabled(eq);
  dither.level = dithering_level();
}

inline size_t AudioProcessor::get_eq_nbands(int ch) const
{ return equalizer.get_nbands(ch); }

inline size_t AudioProcessor::get_eq_bands(int ch, EqBand *bands, size_t first_band, size_t nbands) const
{ return equalizer.get_bands(ch, bands, first_band, nbands); }

inline size_t AudioProcessor::set_eq_bands(int ch, const EqBand *bands, size_t nbands)
{ return equalizer.set_bands(ch, bands, nbands); }

// Bass redirection

inline bool AudioProcessor::get_bass_redir() const
{ return bass_redir.get_enabled(); }

inline int AudioProcessor::get_bass_freq() const
{ return (int)bass_redir.get_freq(); }

inline void AudioProcessor::set_bass_redir(bool _bass_redir)
{ bass_redir.set_enabled(_bass_redir); }

inline void AudioProcessor::set_bass_freq(int _bass_freq)
{ bass_redir.set_freq(_bass_freq); }

// Delays

inline bool AudioProcessor::get_delay() const
{ return delay.get_enabled(); }

inline void AudioProcessor::set_delay(bool _delay)
{ delay.set_enabled(_delay); }

inline int AudioProcessor::get_delay_units() const
{ return delay.get_units(); }

inline void AudioProcessor::get_delays(float _delays[NCHANNELS]) const
{ delay.get_delays(_delays); }

inline void AudioProcessor::set_delay_units(int _delay_units)
{ delay.set_units(_delay_units); }

inline void AudioProcessor::set_delays(const float _delays[NCHANNELS])
{ delay.set_delays(_delays); }

inline void AudioProcessor::set_dbpb(int _dbpb)
{
  in_levels.set_dbpb(_dbpb);
  out_levels.set_dbpb(_dbpb);
}

// Dithering

inline int AudioProcessor::get_dithering() const
{ return dithering; }

inline void AudioProcessor::set_dithering(int _dithering)
{
  dithering = _dithering;
  dither.level = dithering_level();
}

// Cache

inline vtime_t AudioProcessor::get_input_cache_size() const
{ return in_cache.get_size(); }

inline vtime_t AudioProcessor::get_output_cache_size() const
{ return out_cache.get_size(); }

inline void AudioProcessor::set_input_cache_size(vtime_t size)
{ in_cache.set_size(size); }

inline void AudioProcessor::set_output_cache_size(vtime_t size)
{ out_cache.set_size(size); }

inline vtime_t AudioProcessor::get_input_cache_time() const
{ return in_cache.get_time(); }

inline vtime_t AudioProcessor::get_output_cache_time() const
{ return out_cache.get_time(); }

inline size_t AudioProcessor::get_input_cache(int ch_name, vtime_t time, sample_t *buf, size_t size)
{ return in_cache.get_samples(ch_name, time, buf, size); }

inline size_t AudioProcessor::get_output_cache(int ch_name, vtime_t time, sample_t *buf, size_t size)
{ return out_cache.get_samples(ch_name, time, buf, size); }

// Histogram

inline int AudioProcessor::get_dbpb() const
{ return in_levels.get_dbpb(); }

inline void AudioProcessor::get_input_histogram(double *_input_histogram, size_t _count) const
{ in_levels.get_histogram(_input_histogram, _count); }

inline void AudioProcessor::get_input_histogram(int _ch, double *_input_histogram, size_t _count) const
{ in_levels.get_histogram(_ch, _input_histogram, _count); }

inline void AudioProcessor::get_output_histogram(double *_output_histogram, size_t _count) const
{ out_levels.get_histogram(_output_histogram, _count); }

inline void AudioProcessor::get_output_histogram(int _ch, double *_output_histogram, size_t _count) const
{ out_levels.get_histogram(_ch, _output_histogram, _count); }

inline sample_t AudioProcessor::get_max_level() const
{ return out_levels.get_max_level(); }

inline sample_t AudioProcessor::get_max_level(int ch) const
{ return out_levels.get_max_level(ch); }

}; // namespace AudioFilter

#endif

// vim: ts=2 sts=2 et

