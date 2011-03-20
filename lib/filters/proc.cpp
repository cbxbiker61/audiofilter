#include "proc.h"

static const int format_mask = 
  FORMAT_MASK_LINEAR | 
  FORMAT_MASK_PCM16 | FORMAT_MASK_PCM24 | FORMAT_MASK_PCM32 |
  FORMAT_MASK_PCM16_BE | FORMAT_MASK_PCM24_BE | FORMAT_MASK_PCM32_BE | 
  FORMAT_MASK_PCMFLOAT | FORMAT_MASK_PCMDOUBLE |
  FORMAT_MASK_LPCM20 | FORMAT_MASK_LPCM24;

AudioProcessor::AudioProcessor(size_t _nsamples)
:in_conv(_nsamples), mixer(_nsamples), agc(_nsamples), out_conv(_nsamples)
{
  dithering = DITHER_AUTO;
  user_spk = spk_unknown;
  rebuild_chain();
}

size_t
AudioProcessor::get_info(char *_buf, size_t _len) const
{
  return chain.chain_text(_buf, _len);
}


bool 
AudioProcessor::query_user(Speakers _user_spk) const
{
  return user_spk.is_unknown() || (FORMAT_MASK(_user_spk.format) & format_mask) != 0;
}

bool 
AudioProcessor::set_user(Speakers _user_spk)
{
  if (user_spk != _user_spk)
  {
    if (!query_user(_user_spk))
      return false;

    user_spk = _user_spk;
    if (!rebuild_chain())
    {
      in_spk = spk_unknown;
      out_spk = spk_unknown;
      return false;
    }
  }
  return true;
}

Speakers
AudioProcessor::get_user() const
{
  return user_spk;
}

Speakers 
AudioProcessor::user2output(Speakers _in_spk, Speakers _user_spk) const
{
  if (!query_input(_in_spk) || !query_user(_user_spk))
    return spk_unknown;

  Speakers result = _in_spk;
  if (_user_spk.format != FORMAT_UNKNOWN)
  {
    result.format = _user_spk.format;
    result.level = _user_spk.level;
  }

  if (_user_spk.mask)
    result.mask = _user_spk.mask;

  if (_user_spk.sample_rate)
    result.sample_rate = _user_spk.sample_rate;

  result.relation = _user_spk.relation;

  return result;
}

double
AudioProcessor::dithering_level() const
{
  // Do not apply dithering when explicitly disabled
  if (dithering == DITHER_NONE)
    return 0;

  // Do not apply dithering to floating-point output
  if (out_spk.is_floating_point() || out_spk.format == FORMAT_LINEAR)
    return 0;

  bool use_dither = false;

  if (dithering == DITHER_ALWAYS)
    use_dither = true;

  if (dithering == DITHER_AUTO)
  {
    // sample size degrade
    if (out_spk.level < in_spk.level)
      use_dither = true;

    // floating-point to integer conversion
    if (in_spk.is_floating_point() || in_spk.format == FORMAT_LINEAR)
      use_dither = true;

    // sample rate conversion
    if (user_spk.sample_rate && in_spk.sample_rate != user_spk.sample_rate)
      use_dither = true;

    // equalizer
    if (equalizer.get_enabled())
      use_dither = true;
  }

  // Return dithering level
  if (use_dither && out_spk.level > 0)
    return 0.5 / out_spk.level;
  else
    return 0.0;
}


bool 
AudioProcessor::rebuild_chain()
{
  chain.drop();
  if (in_spk.is_unknown())
    return true;

  // Output configuration
  out_spk = user2output(in_spk, user_spk);
  if (out_spk.is_unknown())
    return false;

  // processing chain
  FILTER_SAFE(chain.add_back(&in_levels, "Input levels"));
  FILTER_SAFE(chain.add_back(&in_cache, "Input cache"));
  if (out_spk.nch() < in_spk.nch())
  {
    FILTER_SAFE(chain.add_back(&mixer,     "Mixer"));
    FILTER_SAFE(chain.add_back(&resample,  "SRC"));
  }
  else
  {
    FILTER_SAFE(chain.add_back(&resample,  "SRC"));
    FILTER_SAFE(chain.add_back(&mixer,     "Mixer"));
  }
  FILTER_SAFE(chain.add_back(&bass_redir,"Bass redirection"));
  FILTER_SAFE(chain.add_back(&equalizer, "Equalizer"));
  FILTER_SAFE(chain.add_back(&dither,    "Dither"));
  FILTER_SAFE(chain.add_back(&agc,       "AGC"));
  FILTER_SAFE(chain.add_back(&delay,     "Delay"));
  FILTER_SAFE(chain.add_back(&out_cache, "Output cache"));
  FILTER_SAFE(chain.add_back(&out_levels,"Output levels"));

  // setup mixer
  Speakers mixer_spk = out_spk;
  mixer_spk.format = FORMAT_LINEAR;
  FILTER_SAFE(mixer.set_output(mixer_spk));

  // setup src
  resample.set_sample_rate(user_spk.sample_rate);

  // format conversion
  if (in_spk.format != FORMAT_LINEAR)
  {
    FILTER_SAFE(chain.add_front(&in_conv, "PCM->Linear converter"));
    FILTER_SAFE(in_conv.set_format(FORMAT_LINEAR));
  }

  if (out_spk.format != FORMAT_LINEAR)
  {
    FILTER_SAFE(chain.add_back(&out_conv, "Linear->PCM converter"));
    FILTER_SAFE(out_conv.set_format(out_spk.format));
  }

  dither.level = dithering_level();

  FILTER_SAFE(chain.set_input(in_spk));
  return true;
}

// Filter interface

void 
AudioProcessor::reset()
{
  in_levels.reset();
  mixer.reset();
  resample.reset();
  bass_redir.reset();
  agc.reset();
  delay.reset();
  out_levels.reset();

  chain.reset();
}

bool
AudioProcessor::is_ofdd() const
{
  return false;
}


bool 
AudioProcessor::query_input(Speakers _spk) const
{
  return (FORMAT_MASK(_spk.format) & format_mask) && 
         _spk.sample_rate && 
         _spk.mask;
}

bool 
AudioProcessor::set_input(Speakers _spk)
{
  reset();
  if (!query_input(_spk)) 
    return false;

  in_spk = _spk;
  if (!rebuild_chain())
  {
    in_spk = spk_unknown;
    out_spk = spk_unknown;
    return false;
  }
  return true;
}

Speakers
AudioProcessor::get_input() const
{
  return chain.get_input();
}

bool 
AudioProcessor::process(const Chunk *_chunk)
{
  if (_chunk->is_dummy())
    return true;

  if (_chunk->spk != in_spk && !set_input(_chunk->spk))
    return false;
  else
    return chain.process(_chunk);
}

Speakers 
AudioProcessor::get_output() const
{
  return out_spk;
}

bool 
AudioProcessor::is_empty() const
{
  return chain.is_empty();
}

bool 
AudioProcessor::get_chunk(Chunk *_chunk)
{
  return chain.get_chunk(_chunk);
}

///////////////////////////////////////////////////////////////////////////////

AudioProcessorState *
AudioProcessor::get_state(vtime_t time)
{
  AudioProcessorState *state = new AudioProcessorState;
  if (!state) return 0;

  // Channel order
  get_input_order(state->input_order);
  get_output_order(state->output_order);
  // Master gain
  state->master = get_master();
  state->gain   = get_gain();
  // AGC options
  state->auto_gain = get_auto_gain();
  state->normalize = get_normalize();
  state->attack    = get_attack();
  state->release   = get_release();
  // DRC
  state->drc       = get_drc();
  state->drc_power = get_drc_power();
  state->drc_level = get_drc_level();
  // Matrix
  get_matrix(state->matrix);
  // Automatrix options
  state->auto_matrix      = get_auto_matrix();
  state->normalize_matrix = get_normalize_matrix();
  state->voice_control    = get_voice_control();
  state->expand_stereo    = get_expand_stereo();
  // Automatrix levels
  state->clev = get_clev();
  state->slev = get_slev();
  state->lfelev = get_lfelev();
  // Input/output gains
  get_input_gains(state->input_gains);
  get_output_gains(state->output_gains);
  // Input/output levels
  get_input_levels(time, state->input_levels);
  get_output_levels(time, state->output_levels);
  // SRC
  state->src_quality = get_src_quality();
  state->src_att     = get_src_att();
  // Equalizer
  state->eq = get_eq();
  state->eq_master_nbands = get_eq_nbands(CH_NONE);
  state->eq_master_bands = 0;
  if (state->eq_master_nbands)
  {
    state->eq_master_bands = new EqBand[state->eq_master_nbands];
    get_eq_bands(CH_NONE, state->eq_master_bands, 0, state->eq_master_nbands);
  }

  for (int ch = 0; ch < NCHANNELS; ch++)
  {
    state->eq_nbands[ch] = get_eq_nbands(ch);
    state->eq_bands[ch] = 0;
    if (state->eq_nbands[ch])
    {
      state->eq_bands[ch] = new EqBand[state->eq_nbands[ch]];
      get_eq_bands(ch, state->eq_bands[ch], 0, state->eq_nbands[ch]);
    }
  }

  // Bass redirection
  state->bass_redir = get_bass_redir();
  state->bass_freq = get_bass_freq();
  // Delays
  state->delay = get_delay();
  state->delay_units = get_delay_units();
  get_delays(state->delays);
  // Dithering
  state->dithering = get_dithering();

  return state;
}

void
AudioProcessor::set_state(const AudioProcessorState *state)
{
  if (!state) return;

  // Channel order
  set_input_order(state->input_order);
  set_output_order(state->output_order);
  // Master gain
  set_master(state->master);
  // AGC
  set_auto_gain(state->auto_gain);
  set_normalize(state->normalize);
  set_attack(state->attack);
  set_release(state->release);
  // DRC
  set_drc(state->drc);
  set_drc_power(state->drc_power);
  // Matrix
  // (!) Auto matrix option must be set before setting the matrix
  // because when auto matrix is on, mixer rejects the new matrix.
  set_auto_matrix(state->auto_matrix);
  set_matrix(state->matrix);
  // Automatrix options
  set_normalize_matrix(state->normalize_matrix);
  set_voice_control(state->voice_control);
  set_expand_stereo(state->expand_stereo);
  // Automatrix levels
  set_clev(state->clev);
  set_slev(state->slev);
  set_lfelev(state->lfelev);
  // Input/output gains
  set_input_gains(state->input_gains);
  set_output_gains(state->output_gains);
  // SRC
  set_src_quality(state->src_quality);
  set_src_att(state->src_att);
  // Eqalizer
  set_eq(state->eq);
  if (state->eq_master_bands)
    set_eq_bands(CH_NONE, state->eq_master_bands, state->eq_master_nbands);
  for (int ch = 0; ch < NCHANNELS; ch++)
    if (state->eq_bands[ch])
      set_eq_bands(ch, state->eq_bands[ch], state->eq_nbands[ch]);
  // Bass redirection
  set_bass_redir(state->bass_redir);
  set_bass_freq(state->bass_freq);
  // Delays
  set_delay(state->delay);
  set_delay_units(state->delay_units);
  set_delays(state->delays);
  // Dithering
  set_dithering(state->dithering);
}
