/*
  Matrix Mixer filter
  Apply matrix conversion 
    O = M * I 
  where 
    O - output sample vector [output_channels]
    I - input sample vector [input_channels]
    M - conversion matrix [output_channels, input_channels]
  
  Speakers: can change mask
  Input formats:  Linear
  Buffering: yes/no
  Timing: unchanged
  Parameters:
    output           - output speakers config
    buffer_size      - internal buffer size
    matrix           - conversion matrix
    auto_matrix      - update matrix automatically
    normalize_matrix - normalize matrix so output gain <= 1.0 (matrix parameter)
    voice_control    - allow center gain change even if it is no center channel at input (matrix parameter)
    expand_stereo    - allow surround gain change even if it is no surround channels at input (matrix parameter)
    clev             - center mix level (matrix parameter)
    slev             - surround mix level (matrix parameter)
    lfelev           - LFE mix level (matrix parameter)
    gain             - global gain (can change independently of matrix)
    input_gains      - input channel's gains (can change independently of matrix)
    output_gain      - output channel's gains (can change independently of matrix)
*/

#ifndef VALIB_MIXER_H
#define VALIB_MIXER_H

#include "../buffer.h"
#include "../filter.h"


//typedef sample_t matrix_t[NCHANNELS][NCHANNELS];

///////////////////////////////////////////////////////////////////////////////
// Mixer class
///////////////////////////////////////////////////////////////////////////////

class Mixer : public NullFilter
{
protected:
  // Speakers
  Speakers out_spk;                  // output speakers config

  // Buffer
  SampleBuf buf;                     // sample buffer
  size_t nsamples;                   // buffer size (in samples)
                                  
  // Options                      
  bool     auto_matrix;              // update matrix automatically
  bool     normalize_matrix;         // normalize matrix
  bool     voice_control;            // voice control option
  bool     expand_stereo;            // expand stereo option
                                  
  // Matrix params                
  sample_t clev;                     // center mix level
  sample_t slev;                     // surround mix level
  sample_t lfelev;                   // lfe mix level

  // Gains
  sample_t gain;                     // general gain
  sample_t input_gains[NCHANNELS];   // input channel gains
  sample_t output_gains[NCHANNELS];  // output channel gains

  // Matrix
  matrix_t matrix;                   // mixing matrix
  matrix_t m;                        // reordered mixing matrix (internal)

  void prepare_matrix();

public:
  Mixer(size_t nsamples);

  /////////////////////////////////////////////////////////
  // Filter interface

  virtual bool set_input(Speakers spk);
  virtual Speakers get_output() const;
  virtual bool get_chunk(Chunk *out);

  /////////////////////////////////////////////////////////
  // Mixer interface

  // output format
  bool set_output(Speakers spk);

  // buffer size
  inline bool   is_buffered() const;
  inline size_t get_buffer() const;
  inline void   set_buffer(size_t nsamples);

  // matrix calculation
  void calc_matrix();

  // options get/set
  inline void     get_matrix(matrix_t &matrix) const;
  inline bool     get_auto_matrix() const;
  inline bool     get_normalize_matrix() const;
  inline bool     get_voice_control() const;
  inline bool     get_expand_stereo() const;
  inline sample_t get_clev() const;
  inline sample_t get_slev() const;
  inline sample_t get_lfelev() const;
  inline sample_t get_gain() const;
  inline void     get_input_gains(sample_t input_gains[NCHANNELS]) const;
  inline void     get_output_gains(sample_t output_gains[NCHANNELS]) const;

  inline void     set_matrix(const matrix_t &matrix);
  inline void     set_auto_matrix(bool auto_matrix);
  inline void     set_normalize_matrix(bool normalize_matrix);
  inline void     set_voice_control(bool voice_control);
  inline void     set_expand_stereo(bool expand_stereo);
  inline void     set_clev(sample_t clev);
  inline void     set_slev(sample_t slev);
  inline void     set_lfelev(sample_t lfelev);
  inline void     set_gain(sample_t gain);
  inline void     set_input_gains(const sample_t input_gains[NCHANNELS]);
  inline void     set_output_gains(const sample_t output_gains[NCHANNELS]);

  // mixing functions
  void io_mix11(samples_t input, samples_t output, size_t nsamples);
  void io_mix12(samples_t input, samples_t output, size_t nsamples);
  void io_mix13(samples_t input, samples_t output, size_t nsamples);
  void io_mix14(samples_t input, samples_t output, size_t nsamples);
  void io_mix15(samples_t input, samples_t output, size_t nsamples);
  void io_mix16(samples_t input, samples_t output, size_t nsamples);
  void io_mix21(samples_t input, samples_t output, size_t nsamples);
  void io_mix22(samples_t input, samples_t output, size_t nsamples);
  void io_mix23(samples_t input, samples_t output, size_t nsamples);
  void io_mix24(samples_t input, samples_t output, size_t nsamples);
  void io_mix25(samples_t input, samples_t output, size_t nsamples);
  void io_mix26(samples_t input, samples_t output, size_t nsamples);
  void io_mix31(samples_t input, samples_t output, size_t nsamples);
  void io_mix32(samples_t input, samples_t output, size_t nsamples);
  void io_mix33(samples_t input, samples_t output, size_t nsamples);
  void io_mix34(samples_t input, samples_t output, size_t nsamples);
  void io_mix35(samples_t input, samples_t output, size_t nsamples);
  void io_mix36(samples_t input, samples_t output, size_t nsamples);
  void io_mix41(samples_t input, samples_t output, size_t nsamples);
  void io_mix42(samples_t input, samples_t output, size_t nsamples);
  void io_mix43(samples_t input, samples_t output, size_t nsamples);
  void io_mix44(samples_t input, samples_t output, size_t nsamples);
  void io_mix45(samples_t input, samples_t output, size_t nsamples);
  void io_mix46(samples_t input, samples_t output, size_t nsamples);
  void io_mix51(samples_t input, samples_t output, size_t nsamples);
  void io_mix52(samples_t input, samples_t output, size_t nsamples);
  void io_mix53(samples_t input, samples_t output, size_t nsamples);
  void io_mix54(samples_t input, samples_t output, size_t nsamples);
  void io_mix55(samples_t input, samples_t output, size_t nsamples);
  void io_mix56(samples_t input, samples_t output, size_t nsamples);
  void io_mix61(samples_t input, samples_t output, size_t nsamples);
  void io_mix62(samples_t input, samples_t output, size_t nsamples);
  void io_mix63(samples_t input, samples_t output, size_t nsamples);
  void io_mix64(samples_t input, samples_t output, size_t nsamples);
  void io_mix65(samples_t input, samples_t output, size_t nsamples);
  void io_mix66(samples_t input, samples_t output, size_t nsamples);

  void ip_mix11(samples_t samples, size_t nsamples);
  void ip_mix12(samples_t samples, size_t nsamples);
  void ip_mix13(samples_t samples, size_t nsamples);
  void ip_mix14(samples_t samples, size_t nsamples);
  void ip_mix15(samples_t samples, size_t nsamples);
  void ip_mix16(samples_t samples, size_t nsamples);
  void ip_mix21(samples_t samples, size_t nsamples);
  void ip_mix22(samples_t samples, size_t nsamples);
  void ip_mix23(samples_t samples, size_t nsamples);
  void ip_mix24(samples_t samples, size_t nsamples);
  void ip_mix25(samples_t samples, size_t nsamples);
  void ip_mix26(samples_t samples, size_t nsamples);
  void ip_mix31(samples_t samples, size_t nsamples);
  void ip_mix32(samples_t samples, size_t nsamples);
  void ip_mix33(samples_t samples, size_t nsamples);
  void ip_mix34(samples_t samples, size_t nsamples);
  void ip_mix35(samples_t samples, size_t nsamples);
  void ip_mix36(samples_t samples, size_t nsamples);
  void ip_mix41(samples_t samples, size_t nsamples);
  void ip_mix42(samples_t samples, size_t nsamples);
  void ip_mix43(samples_t samples, size_t nsamples);
  void ip_mix44(samples_t samples, size_t nsamples);
  void ip_mix45(samples_t samples, size_t nsamples);
  void ip_mix46(samples_t samples, size_t nsamples);
  void ip_mix51(samples_t samples, size_t nsamples);
  void ip_mix52(samples_t samples, size_t nsamples);
  void ip_mix53(samples_t samples, size_t nsamples);
  void ip_mix54(samples_t samples, size_t nsamples);
  void ip_mix55(samples_t samples, size_t nsamples);
  void ip_mix56(samples_t samples, size_t nsamples);
  void ip_mix61(samples_t samples, size_t nsamples);
  void ip_mix62(samples_t samples, size_t nsamples);
  void ip_mix63(samples_t samples, size_t nsamples);
  void ip_mix64(samples_t samples, size_t nsamples);
  void ip_mix65(samples_t samples, size_t nsamples);
  void ip_mix66(samples_t samples, size_t nsamples);
};

///////////////////////////////////////////////////////////////////////////////
// Mixer inlines
///////////////////////////////////////////////////////////////////////////////

// Buffer management

inline bool
Mixer::is_buffered() const
{
  return out_spk.nch() > spk.nch();
}

inline size_t
Mixer::get_buffer() const
{
  return nsamples;
}

inline void 
Mixer::set_buffer(size_t _nsamples)
{
  nsamples = _nsamples;
  if (is_buffered())
    buf.allocate(out_spk.nch(), _nsamples);
}

// Options get/set

inline void
Mixer::get_matrix(matrix_t &_matrix) const
{ _matrix = matrix; }

inline bool 
Mixer::get_auto_matrix() const
{ return auto_matrix; }

inline bool 
Mixer::get_normalize_matrix() const
{ return normalize_matrix; }

inline bool 
Mixer::get_voice_control() const
{ return voice_control; }

inline bool 
Mixer::get_expand_stereo() const
{ return expand_stereo; }

inline sample_t 
Mixer::get_clev() const
{ return clev; }

inline sample_t 
Mixer::get_slev() const
{ return slev; }

inline sample_t 
Mixer::get_lfelev() const
{ return lfelev; }

inline sample_t 
Mixer::get_gain() const
{ return gain; }

inline void 
Mixer::get_input_gains(sample_t _input_gains[NCHANNELS]) const
{ memcpy(_input_gains, input_gains, sizeof(input_gains)); }

inline void 
Mixer::get_output_gains(sample_t _output_gains[NCHANNELS]) const
{ memcpy(_output_gains, output_gains, sizeof(output_gains)); }


inline void 
Mixer::set_matrix(const matrix_t &_matrix)
{
  if (!auto_matrix)
  {
    matrix = _matrix;
    prepare_matrix();
  }
}

inline void 
Mixer::set_auto_matrix(bool _auto_matrix)
{
  auto_matrix = _auto_matrix;
  if (auto_matrix) calc_matrix();
}

inline void 
Mixer::set_normalize_matrix(bool _normalize_matrix)
{
  normalize_matrix = _normalize_matrix;
  if (auto_matrix) calc_matrix();
}

inline void 
Mixer::set_voice_control(bool _voice_control)
{
  voice_control = _voice_control;
  if (auto_matrix) calc_matrix();
}

inline void 
Mixer::set_expand_stereo(bool _expand_stereo)
{
  expand_stereo = _expand_stereo;
  if (auto_matrix) calc_matrix();
}

inline void 
Mixer::set_clev(sample_t _clev)
{
  clev = _clev;
  if (auto_matrix) calc_matrix();
}

inline void 
Mixer::set_slev(sample_t _slev)
{
  slev = _slev;
  if (auto_matrix) calc_matrix();
}

inline void 
Mixer::set_lfelev(sample_t _lfelev)
{
  lfelev = _lfelev;
  if (auto_matrix) calc_matrix();
}
inline void 
Mixer::set_gain(sample_t _gain)
{
  gain = _gain;
  prepare_matrix();
}

inline void 
Mixer::set_input_gains(const sample_t _input_gains[NCHANNELS])
{
  memcpy(input_gains, _input_gains, sizeof(input_gains));
  prepare_matrix();
}

inline void 
Mixer::set_output_gains(const sample_t _output_gains[NCHANNELS])
{
  memcpy(output_gains, _output_gains, sizeof(output_gains));
  prepare_matrix();
}

#endif
