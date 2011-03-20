#include <math.h>
#include <string.h>
#include "mixer.h"

typedef void (Mixer::*io_mixfunc_t)(samples_t, samples_t, size_t); // input-output mixing
typedef void (Mixer::*ip_mixfunc_t)(samples_t, size_t);            // in-place mixing

static const io_mixfunc_t io_mix_tbl[NCHANNELS][NCHANNELS] = {
  { &Mixer::io_mix11, &Mixer::io_mix12, &Mixer::io_mix13, &Mixer::io_mix14, &Mixer::io_mix15, &Mixer::io_mix16 },
  { &Mixer::io_mix21, &Mixer::io_mix22, &Mixer::io_mix23, &Mixer::io_mix24, &Mixer::io_mix25, &Mixer::io_mix26 },
  { &Mixer::io_mix31, &Mixer::io_mix32, &Mixer::io_mix33, &Mixer::io_mix34, &Mixer::io_mix35, &Mixer::io_mix36 },
  { &Mixer::io_mix41, &Mixer::io_mix42, &Mixer::io_mix43, &Mixer::io_mix44, &Mixer::io_mix45, &Mixer::io_mix46 },
  { &Mixer::io_mix51, &Mixer::io_mix52, &Mixer::io_mix53, &Mixer::io_mix54, &Mixer::io_mix55, &Mixer::io_mix56 },
  { &Mixer::io_mix61, &Mixer::io_mix62, &Mixer::io_mix63, &Mixer::io_mix64, &Mixer::io_mix65, &Mixer::io_mix66 },
};

static const ip_mixfunc_t ip_mix_tbl[NCHANNELS][NCHANNELS] = {
  { &Mixer::ip_mix11, &Mixer::ip_mix12, &Mixer::ip_mix13, &Mixer::ip_mix14, &Mixer::ip_mix15, &Mixer::ip_mix16 },
  { &Mixer::ip_mix21, &Mixer::ip_mix22, &Mixer::ip_mix23, &Mixer::ip_mix24, &Mixer::ip_mix25, &Mixer::ip_mix26 },
  { &Mixer::ip_mix31, &Mixer::ip_mix32, &Mixer::ip_mix33, &Mixer::ip_mix34, &Mixer::ip_mix35, &Mixer::ip_mix36 },
  { &Mixer::ip_mix41, &Mixer::ip_mix42, &Mixer::ip_mix43, &Mixer::ip_mix44, &Mixer::ip_mix45, &Mixer::ip_mix46 },
  { &Mixer::ip_mix51, &Mixer::ip_mix52, &Mixer::ip_mix53, &Mixer::ip_mix54, &Mixer::ip_mix55, &Mixer::ip_mix56 },
  { &Mixer::ip_mix61, &Mixer::ip_mix62, &Mixer::ip_mix63, &Mixer::ip_mix64, &Mixer::ip_mix65, &Mixer::ip_mix66 },
};

Mixer::Mixer(size_t _nsamples)
:NullFilter(FORMAT_MASK_LINEAR)
{
  nsamples = _nsamples;
  out_spk = spk_unknown;

  // Options
  auto_matrix      = true;
  normalize_matrix = true;
  voice_control    = true;
  expand_stereo    = true;

  // Matrix params
  clev   = 1.0;
  slev   = 1.0;
  lfelev = 1.0;

  // Gains
  gain = 1.0;
  for (int ch = 0; ch < NCHANNELS; ch++)
  {
    input_gains[ch]  = 1.0;
    output_gains[ch] = 1.0;
  }

  // Matrix
  calc_matrix();

  // We don't allocate sample buffer 
  // because we may not need it
}



void 
Mixer::prepare_matrix()
{
  // Convert input matrix into internal form
  // to achieve maximum performance

  // todo: throw out null columns
  // todo: zero null output channels instead of matrixing 
  // todo: do not touch pass-through channels
  // todo: gain channel if possible instead of matrixing

  const int *in_order = spk.order();
  const int *out_order = out_spk.order();
  sample_t factor = 1.0;

  if (spk.level > 0.0)
    factor = out_spk.level / spk.level * gain;
  else
    factor = out_spk.level * gain;

  for (int ch1 = 0; ch1 < spk.nch(); ch1++)
    for (int ch2 = 0; ch2 < out_spk.nch(); ch2++)
      m[ch1][ch2] = 
        matrix[in_order[ch1]][out_order[ch2]] * 
        input_gains[in_order[ch1]] * 
        output_gains[out_order[ch2]] * 
        factor;
}

bool
Mixer::set_output(Speakers _spk)
{
  if (!query_input(_spk)) 
    return false;

  out_spk = _spk;
  out_spk.sample_rate = spk.sample_rate;

  if (is_buffered())
    buf.allocate(out_spk.nch(), nsamples);

  if (auto_matrix)
    calc_matrix();

  prepare_matrix();

  return true;
}

///////////////////////////////////////////////////////////
// Filter interface

bool 
Mixer::set_input(Speakers _spk)
{
  if (!NullFilter::set_input(_spk))
    return false;

  out_spk.sample_rate = spk.sample_rate;

  if (is_buffered())
    buf.allocate(out_spk.nch(), nsamples);

  if (auto_matrix)
    calc_matrix();

  prepare_matrix();

  return true;
}

Speakers 
Mixer::get_output() const
{
  return out_spk;
}


bool 
Mixer::get_chunk(Chunk *_chunk)
{
  if (is_buffered())
  {
    // buffered mixing
    size_t n = MIN(nsamples, size);
    io_mixfunc_t mixfunc = io_mix_tbl[spk.nch()-1][out_spk.nch()-1];
    (this->*mixfunc)(samples, buf, n);
    samples += n;
    size -= n;

    // fill output chunk
    _chunk->set_linear
    (
      out_spk,
      buf, n,
      sync, time, 
      flushing && !size
    );

    sync = false;
    flushing = flushing && size;
  }
  else
  {
    // in-place mixing
    ip_mixfunc_t mixfunc = ip_mix_tbl[spk.nch()-1][out_spk.nch()-1];
    (this->*mixfunc)(samples, size);

    // fill output chunk
    _chunk->set_linear
    (
      out_spk,
      samples, size,
      sync, time,
      flushing
    );

    size = 0;
    sync = false;
    flushing = false;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// Matrix calculation
///////////////////////////////////////////////////////////////////////////////

void 
Mixer::calc_matrix()
{
  int in_mask    = spk.mask;
  int out_mask   = out_spk.mask;

  int in_nfront  = ((in_mask >> CH_L)  & 1) + ((in_mask >> CH_C)  & 1) + ((in_mask >> CH_R) & 1);
  int in_nrear   = ((in_mask >> CH_SL) & 1) + ((in_mask >> CH_SR) & 1);

  int out_nfront = ((out_mask >> CH_L)  & 1) + ((out_mask >> CH_C)  & 1) + ((out_mask >> CH_R) & 1);
  int out_nrear  = ((out_mask >> CH_SL) & 1) + ((out_mask >> CH_SR) & 1);

  int in_dolby  = NO_RELATION;
  int out_dolby = NO_RELATION;

  if (spk.relation == RELATION_DOLBY ||
      spk.relation == RELATION_DOLBY2)
    in_dolby = spk.relation;

  if (out_spk.relation == RELATION_DOLBY ||
      out_spk.relation == RELATION_DOLBY2)
    out_dolby = out_spk.relation;
  
  for (int i = 0; i < NCHANNELS; i++)
    for (int j = 0; j < NCHANNELS; j++)
      matrix[i][j] = 0;

  // Dolby modes are backwards-compatible
  if (in_dolby && out_dolby)
  {
    matrix[CH_L][CH_L] = 1;
    matrix[CH_R][CH_R] = 1;
  }
  // Mix to Dolby Surround/ProLogic/ProLogicII
  else if (out_dolby)
  {
    if (in_nfront >= 2)
    {
      matrix[CH_L][CH_L] = 1;
      matrix[CH_R][CH_R] = 1;
    }
    if (in_nfront != 2)
    {
      matrix[CH_C][CH_L] = LEVEL_3DB * clev;
      matrix[CH_C][CH_R] = LEVEL_3DB * clev;
    }
    if (in_nrear == 1)
    {
      matrix[CH_S][CH_L] = -LEVEL_3DB * slev;
      matrix[CH_S][CH_R] = +LEVEL_3DB * slev;
    }
    else if (in_nrear == 2)
    {
      switch (out_dolby)
      { 
        case RELATION_DOLBY2:
          matrix[CH_SL][CH_L] = -0.8660*slev;
          matrix[CH_SR][CH_L] = -0.5000*slev;
          matrix[CH_SL][CH_R] = +0.5000*slev;
          matrix[CH_SR][CH_R] = +0.8660*slev;
          break;

        case RELATION_DOLBY:
        default:
          matrix[CH_SL][CH_L] = -slev;
          matrix[CH_SR][CH_L] = -slev;
          matrix[CH_SL][CH_R] = +slev;
          matrix[CH_SR][CH_R] = +slev;
          break;
      }
    }
  }
  else
  {
    // direct route equal channels
    if (in_mask & out_mask & CH_MASK_L)  matrix[CH_L] [CH_L]  = 1.0;
    if (in_mask & out_mask & CH_MASK_R)  matrix[CH_R] [CH_R]  = 1.0;
    if (in_mask & out_mask & CH_MASK_C)  matrix[CH_C] [CH_C]  = clev;
    if (in_mask & out_mask & CH_MASK_SL) matrix[CH_SL][CH_SL] = slev;
    if (in_mask & out_mask & CH_MASK_SR) matrix[CH_SR][CH_SR] = slev;

    // mix front channels
    if (out_nfront == 1 && in_nfront > 1)
    {
      matrix[CH_L][CH_M] = 1;
      matrix[CH_R][CH_M] = 1;
    }
    if (out_nfront == 2 && in_nfront != 2)
    {
      matrix[CH_C][CH_L] = LEVEL_3DB * clev;
      matrix[CH_C][CH_R] = LEVEL_3DB * clev;
    }

    // mix rear into front channels
    if (out_nrear == 0)
    {
      if (in_nrear == 1 && out_nfront == 1)
      {
        matrix[CH_S][CH_M] = clev;
      }
      if (in_nrear == 1 && out_nfront != 1)
      {
        matrix[CH_S][CH_L] = LEVEL_3DB * slev;
        matrix[CH_S][CH_R] = LEVEL_3DB * slev;
      }
      if (in_nrear == 2 && out_nfront == 1)
      {
        matrix[CH_SL][CH_M] = slev;
        matrix[CH_SR][CH_M] = slev;
      }
      if (in_nrear == 2 && out_nfront != 1)
      {
        matrix[CH_SL][CH_L] = slev;
        matrix[CH_SR][CH_R] = slev;
      }
    }

    // mix rear channels
    if (out_nrear == 1 && in_nrear == 2)
    {
      matrix[CH_SL][CH_S] = slev;
      matrix[CH_SR][CH_S] = slev;
    }
    if (out_nrear == 2 && in_nrear == 1)
    {
      matrix[CH_S][CH_SL] = LEVEL_3DB * slev;
      matrix[CH_S][CH_SR] = LEVEL_3DB * slev;
    }
  }

  // Expand stereo & Voice control
  bool expand_stereo_allowed = expand_stereo && !in_nrear;
  bool voice_control_allowed = voice_control && (in_nfront == 2);

  if ((voice_control_allowed || expand_stereo_allowed) && !out_dolby)
  {
    if (voice_control_allowed && out_nfront != 2)
    {
      // C' = clev * (L + R) * LEVEL_3DB
      matrix[CH_L][CH_C] = clev * LEVEL_3DB;
      matrix[CH_R][CH_C] = clev * LEVEL_3DB;
    }

    if (expand_stereo_allowed && in_nfront == 2 && out_nrear)
    {
      if (out_nrear == 1)
      {
        // S' = slev * (L - R)
        matrix[CH_L][CH_S] = + slev;
        matrix[CH_R][CH_S] = - slev;
      }
      if (out_nrear == 2)
      {
        // SL' = slev * 1/2 (L - R)
        // SR' = slev * 1/2 (R - L)
        matrix[CH_L][CH_SL] = + 0.5 * slev;
        matrix[CH_R][CH_SL] = - 0.5 * slev;
        matrix[CH_L][CH_SR] = - 0.5 * slev;
        matrix[CH_R][CH_SR] = + 0.5 * slev;
      }
    }

    if (in_nfront != 1)
    {
      if (expand_stereo_allowed && voice_control_allowed)
      {
        // L' = L * 1/2 (slev + clev) - R * 1/2 (slev - clev)
        // R' = R * 1/2 (slev + clev) - L * 1/2 (slev - clev)
        matrix[CH_L][CH_L] = + 0.5 * (slev + clev);
        matrix[CH_R][CH_L] = - 0.5 * (slev - clev);
        matrix[CH_L][CH_R] = - 0.5 * (slev - clev);
        matrix[CH_R][CH_R] = + 0.5 * (slev + clev);
      }
      else if (expand_stereo_allowed)
      {
        matrix[CH_L][CH_L] = + 0.5 * (slev + 1);
        matrix[CH_R][CH_L] = - 0.5 * (slev - 1);
        matrix[CH_L][CH_R] = - 0.5 * (slev - 1);
        matrix[CH_R][CH_R] = + 0.5 * (slev + 1);
      }
      else // if (voice_control_allowed)
      {
        matrix[CH_L][CH_L] = + 0.5 * (1 + clev);
        matrix[CH_R][CH_L] = - 0.5 * (1 - clev);
        matrix[CH_L][CH_R] = - 0.5 * (1 - clev);
        matrix[CH_R][CH_R] = + 0.5 * (1 + clev);
      }
    }
  }

  // Mix LFE channel

  if (in_mask & out_mask & CH_MASK_LFE) 
     matrix[CH_LFE][CH_LFE] = lfelev;

  if (in_mask & ~out_mask & CH_MASK_LFE)
  {
    // To preserve the resulting loudness, we should apply sqrt(N) gain when 
    // mixing LFE channel (N is number of output channels we mix LFE to).

    double lfenorm = 0;
    if (out_spk.mask & CH_MASK_L)  lfenorm += 1;
    if (out_spk.mask & CH_MASK_R)  lfenorm += 1;
    if (out_spk.mask & CH_MASK_SL) lfenorm += 1;
    if (out_spk.mask & CH_MASK_SR) lfenorm += 1;

    if (lfenorm > 0)
    {
      lfenorm = 1.0 / sqrt(lfenorm);
      if (out_spk.mask & CH_MASK_L)  matrix[CH_LFE][CH_L]  = lfenorm * lfelev;
      if (out_spk.mask & CH_MASK_R)  matrix[CH_LFE][CH_R]  = lfenorm * lfelev;
      if (out_spk.mask & CH_MASK_SL) matrix[CH_LFE][CH_SL] = lfenorm * lfelev;
      if (out_spk.mask & CH_MASK_SR) matrix[CH_LFE][CH_SR] = lfenorm * lfelev;
    }

    // Mix LFE to the center channel only when it is the only output channel

    if (in_mask & CH_MASK_C && out_nfront == 1)  
      matrix[CH_LFE][CH_C]  = lfelev;
  }

  if (normalize_matrix)
  {
    double levels[NCHANNELS] = { 0, 0, 0, 0, 0, 0 };
    double max_level;
    double norm;
    int i, j;

    for (i = 0; i < NCHANNELS; i++)
      for (j = 0; j < NCHANNELS; j++)
        levels[i] += fabs(matrix[j][i]);

    max_level = levels[0];
    for (i = 1; i < NCHANNELS; i++)
      if (levels[i] > max_level) 
        max_level = levels[i];

    if (max_level > 0)
      norm = 1.0/max_level;
    else
      norm = 1.0;

    for (i = 0; i < NCHANNELS; i++)
      for (j = 0; j < NCHANNELS; j++)
        matrix[j][i] *= norm;
  }

  prepare_matrix();
}



///////////////////////////////////////////////////////////////////////////////
// Mixing functions
///////////////////////////////////////////////////////////////////////////////

void Mixer::io_mix11(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[1];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    output[0][s] = buf[0];
  }
}

void Mixer::io_mix12(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[2];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[1]  = input[0][s] * m[0][1];
    output[0][s] = buf[0];
    output[1][s] = buf[1];
  }
}

void Mixer::io_mix13(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[3];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[1]  = input[0][s] * m[0][1];
    buf[2]  = input[0][s] * m[0][2];
    output[0][s] = buf[0];
    output[1][s] = buf[1];
    output[2][s] = buf[2];
  }
}

void Mixer::io_mix14(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[4];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[1]  = input[0][s] * m[0][1];
    buf[2]  = input[0][s] * m[0][2];
    buf[3]  = input[0][s] * m[0][3];
    output[0][s] = buf[0];
    output[1][s] = buf[1];
    output[2][s] = buf[2];
    output[3][s] = buf[3];
  }
}

void Mixer::io_mix15(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[5];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[1]  = input[0][s] * m[0][1];
    buf[2]  = input[0][s] * m[0][2];
    buf[3]  = input[0][s] * m[0][3];
    buf[4]  = input[0][s] * m[0][4];
    output[0][s] = buf[0];
    output[1][s] = buf[1];
    output[2][s] = buf[2];
    output[3][s] = buf[3];
    output[4][s] = buf[4];
  }
}

void Mixer::io_mix16(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[6];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[1]  = input[0][s] * m[0][1];
    buf[2]  = input[0][s] * m[0][2];
    buf[3]  = input[0][s] * m[0][3];
    buf[4]  = input[0][s] * m[0][4];
    buf[5]  = input[0][s] * m[0][5];
    output[0][s] = buf[0];
    output[1][s] = buf[1];
    output[2][s] = buf[2];
    output[3][s] = buf[3];
    output[4][s] = buf[4];
    output[5][s] = buf[5];
  }
}

void Mixer::io_mix21(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[1];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[0] += input[1][s] * m[1][0];
    output[0][s] = buf[0];
  }
}

void Mixer::io_mix22(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[2];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[0] += input[1][s] * m[1][0];
    buf[1]  = input[0][s] * m[0][1];
    buf[1] += input[1][s] * m[1][1];
    output[0][s] = buf[0];
    output[1][s] = buf[1];
  }
}

void Mixer::io_mix23(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[3];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[0] += input[1][s] * m[1][0];
    buf[1]  = input[0][s] * m[0][1];
    buf[1] += input[1][s] * m[1][1];
    buf[2]  = input[0][s] * m[0][2];
    buf[2] += input[1][s] * m[1][2];
    output[0][s] = buf[0];
    output[1][s] = buf[1];
    output[2][s] = buf[2];
  }
}

void Mixer::io_mix24(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[4];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[0] += input[1][s] * m[1][0];
    buf[1]  = input[0][s] * m[0][1];
    buf[1] += input[1][s] * m[1][1];
    buf[2]  = input[0][s] * m[0][2];
    buf[2] += input[1][s] * m[1][2];
    buf[3]  = input[0][s] * m[0][3];
    buf[3] += input[1][s] * m[1][3];
    output[0][s] = buf[0];
    output[1][s] = buf[1];
    output[2][s] = buf[2];
    output[3][s] = buf[3];
  }
}

void Mixer::io_mix25(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[5];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[0] += input[1][s] * m[1][0];
    buf[1]  = input[0][s] * m[0][1];
    buf[1] += input[1][s] * m[1][1];
    buf[2]  = input[0][s] * m[0][2];
    buf[2] += input[1][s] * m[1][2];
    buf[3]  = input[0][s] * m[0][3];
    buf[3] += input[1][s] * m[1][3];
    buf[4]  = input[0][s] * m[0][4];
    buf[4] += input[1][s] * m[1][4];
    output[0][s] = buf[0];
    output[1][s] = buf[1];
    output[2][s] = buf[2];
    output[3][s] = buf[3];
    output[4][s] = buf[4];
  }
}

void Mixer::io_mix26(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[6];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[0] += input[1][s] * m[1][0];
    buf[1]  = input[0][s] * m[0][1];
    buf[1] += input[1][s] * m[1][1];
    buf[2]  = input[0][s] * m[0][2];
    buf[2] += input[1][s] * m[1][2];
    buf[3]  = input[0][s] * m[0][3];
    buf[3] += input[1][s] * m[1][3];
    buf[4]  = input[0][s] * m[0][4];
    buf[4] += input[1][s] * m[1][4];
    buf[5]  = input[0][s] * m[0][5];
    buf[5] += input[1][s] * m[1][5];
    output[0][s] = buf[0];
    output[1][s] = buf[1];
    output[2][s] = buf[2];
    output[3][s] = buf[3];
    output[4][s] = buf[4];
    output[5][s] = buf[5];
  }
}

void Mixer::io_mix31(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[1];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[0] += input[1][s] * m[1][0];
    buf[0] += input[2][s] * m[2][0];
    output[0][s] = buf[0];
  }
}

void Mixer::io_mix32(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[2];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[0] += input[1][s] * m[1][0];
    buf[0] += input[2][s] * m[2][0];
    buf[1]  = input[0][s] * m[0][1];
    buf[1] += input[1][s] * m[1][1];
    buf[1] += input[2][s] * m[2][1];
    output[0][s] = buf[0];
    output[1][s] = buf[1];
  }
}

void Mixer::io_mix33(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[3];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[0] += input[1][s] * m[1][0];
    buf[0] += input[2][s] * m[2][0];
    buf[1]  = input[0][s] * m[0][1];
    buf[1] += input[1][s] * m[1][1];
    buf[1] += input[2][s] * m[2][1];
    buf[2]  = input[0][s] * m[0][2];
    buf[2] += input[1][s] * m[1][2];
    buf[2] += input[2][s] * m[2][2];
    output[0][s] = buf[0];
    output[1][s] = buf[1];
    output[2][s] = buf[2];
  }
}

void Mixer::io_mix34(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[4];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[0] += input[1][s] * m[1][0];
    buf[0] += input[2][s] * m[2][0];
    buf[1]  = input[0][s] * m[0][1];
    buf[1] += input[1][s] * m[1][1];
    buf[1] += input[2][s] * m[2][1];
    buf[2]  = input[0][s] * m[0][2];
    buf[2] += input[1][s] * m[1][2];
    buf[2] += input[2][s] * m[2][2];
    buf[3]  = input[0][s] * m[0][3];
    buf[3] += input[1][s] * m[1][3];
    buf[3] += input[2][s] * m[2][3];
    output[0][s] = buf[0];
    output[1][s] = buf[1];
    output[2][s] = buf[2];
    output[3][s] = buf[3];
  }
}

void Mixer::io_mix35(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[5];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[0] += input[1][s] * m[1][0];
    buf[0] += input[2][s] * m[2][0];
    buf[1]  = input[0][s] * m[0][1];
    buf[1] += input[1][s] * m[1][1];
    buf[1] += input[2][s] * m[2][1];
    buf[2]  = input[0][s] * m[0][2];
    buf[2] += input[1][s] * m[1][2];
    buf[2] += input[2][s] * m[2][2];
    buf[3]  = input[0][s] * m[0][3];
    buf[3] += input[1][s] * m[1][3];
    buf[3] += input[2][s] * m[2][3];
    buf[4]  = input[0][s] * m[0][4];
    buf[4] += input[1][s] * m[1][4];
    buf[4] += input[2][s] * m[2][4];
    output[0][s] = buf[0];
    output[1][s] = buf[1];
    output[2][s] = buf[2];
    output[3][s] = buf[3];
    output[4][s] = buf[4];
  }
}

void Mixer::io_mix36(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[6];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[0] += input[1][s] * m[1][0];
    buf[0] += input[2][s] * m[2][0];
    buf[1]  = input[0][s] * m[0][1];
    buf[1] += input[1][s] * m[1][1];
    buf[1] += input[2][s] * m[2][1];
    buf[2]  = input[0][s] * m[0][2];
    buf[2] += input[1][s] * m[1][2];
    buf[2] += input[2][s] * m[2][2];
    buf[3]  = input[0][s] * m[0][3];
    buf[3] += input[1][s] * m[1][3];
    buf[3] += input[2][s] * m[2][3];
    buf[4]  = input[0][s] * m[0][4];
    buf[4] += input[1][s] * m[1][4];
    buf[4] += input[2][s] * m[2][4];
    buf[5]  = input[0][s] * m[0][5];
    buf[5] += input[1][s] * m[1][5];
    buf[5] += input[2][s] * m[2][5];
    output[0][s] = buf[0];
    output[1][s] = buf[1];
    output[2][s] = buf[2];
    output[3][s] = buf[3];
    output[4][s] = buf[4];
    output[5][s] = buf[5];
  }
}

void Mixer::io_mix41(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[1];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[0] += input[1][s] * m[1][0];
    buf[0] += input[2][s] * m[2][0];
    buf[0] += input[3][s] * m[3][0];
    output[0][s] = buf[0];
  }
}

void Mixer::io_mix42(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[2];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[0] += input[1][s] * m[1][0];
    buf[0] += input[2][s] * m[2][0];
    buf[0] += input[3][s] * m[3][0];
    buf[1]  = input[0][s] * m[0][1];
    buf[1] += input[1][s] * m[1][1];
    buf[1] += input[2][s] * m[2][1];
    buf[1] += input[3][s] * m[3][1];
    output[0][s] = buf[0];
    output[1][s] = buf[1];
  }
}

void Mixer::io_mix43(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[3];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[0] += input[1][s] * m[1][0];
    buf[0] += input[2][s] * m[2][0];
    buf[0] += input[3][s] * m[3][0];
    buf[1]  = input[0][s] * m[0][1];
    buf[1] += input[1][s] * m[1][1];
    buf[1] += input[2][s] * m[2][1];
    buf[1] += input[3][s] * m[3][1];
    buf[2]  = input[0][s] * m[0][2];
    buf[2] += input[1][s] * m[1][2];
    buf[2] += input[2][s] * m[2][2];
    buf[2] += input[3][s] * m[3][2];
    output[0][s] = buf[0];
    output[1][s] = buf[1];
    output[2][s] = buf[2];
  }
}

void Mixer::io_mix44(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[4];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[0] += input[1][s] * m[1][0];
    buf[0] += input[2][s] * m[2][0];
    buf[0] += input[3][s] * m[3][0];
    buf[1]  = input[0][s] * m[0][1];
    buf[1] += input[1][s] * m[1][1];
    buf[1] += input[2][s] * m[2][1];
    buf[1] += input[3][s] * m[3][1];
    buf[2]  = input[0][s] * m[0][2];
    buf[2] += input[1][s] * m[1][2];
    buf[2] += input[2][s] * m[2][2];
    buf[2] += input[3][s] * m[3][2];
    buf[3]  = input[0][s] * m[0][3];
    buf[3] += input[1][s] * m[1][3];
    buf[3] += input[2][s] * m[2][3];
    buf[3] += input[3][s] * m[3][3];
    output[0][s] = buf[0];
    output[1][s] = buf[1];
    output[2][s] = buf[2];
    output[3][s] = buf[3];
  }
}

void Mixer::io_mix45(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[5];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[0] += input[1][s] * m[1][0];
    buf[0] += input[2][s] * m[2][0];
    buf[0] += input[3][s] * m[3][0];
    buf[1]  = input[0][s] * m[0][1];
    buf[1] += input[1][s] * m[1][1];
    buf[1] += input[2][s] * m[2][1];
    buf[1] += input[3][s] * m[3][1];
    buf[2]  = input[0][s] * m[0][2];
    buf[2] += input[1][s] * m[1][2];
    buf[2] += input[2][s] * m[2][2];
    buf[2] += input[3][s] * m[3][2];
    buf[3]  = input[0][s] * m[0][3];
    buf[3] += input[1][s] * m[1][3];
    buf[3] += input[2][s] * m[2][3];
    buf[3] += input[3][s] * m[3][3];
    buf[4]  = input[0][s] * m[0][4];
    buf[4] += input[1][s] * m[1][4];
    buf[4] += input[2][s] * m[2][4];
    buf[4] += input[3][s] * m[3][4];
    output[0][s] = buf[0];
    output[1][s] = buf[1];
    output[2][s] = buf[2];
    output[3][s] = buf[3];
    output[4][s] = buf[4];
  }
}

void Mixer::io_mix46(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[6];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[0] += input[1][s] * m[1][0];
    buf[0] += input[2][s] * m[2][0];
    buf[0] += input[3][s] * m[3][0];
    buf[1]  = input[0][s] * m[0][1];
    buf[1] += input[1][s] * m[1][1];
    buf[1] += input[2][s] * m[2][1];
    buf[1] += input[3][s] * m[3][1];
    buf[2]  = input[0][s] * m[0][2];
    buf[2] += input[1][s] * m[1][2];
    buf[2] += input[2][s] * m[2][2];
    buf[2] += input[3][s] * m[3][2];
    buf[3]  = input[0][s] * m[0][3];
    buf[3] += input[1][s] * m[1][3];
    buf[3] += input[2][s] * m[2][3];
    buf[3] += input[3][s] * m[3][3];
    buf[4]  = input[0][s] * m[0][4];
    buf[4] += input[1][s] * m[1][4];
    buf[4] += input[2][s] * m[2][4];
    buf[4] += input[3][s] * m[3][4];
    buf[5]  = input[0][s] * m[0][5];
    buf[5] += input[1][s] * m[1][5];
    buf[5] += input[2][s] * m[2][5];
    buf[5] += input[3][s] * m[3][5];
    output[0][s] = buf[0];
    output[1][s] = buf[1];
    output[2][s] = buf[2];
    output[3][s] = buf[3];
    output[4][s] = buf[4];
    output[5][s] = buf[5];
  }
}

void Mixer::io_mix51(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[1];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[0] += input[1][s] * m[1][0];
    buf[0] += input[2][s] * m[2][0];
    buf[0] += input[3][s] * m[3][0];
    buf[0] += input[4][s] * m[4][0];
    output[0][s] = buf[0];
  }
}

void Mixer::io_mix52(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[2];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[0] += input[1][s] * m[1][0];
    buf[0] += input[2][s] * m[2][0];
    buf[0] += input[3][s] * m[3][0];
    buf[0] += input[4][s] * m[4][0];
    buf[1]  = input[0][s] * m[0][1];
    buf[1] += input[1][s] * m[1][1];
    buf[1] += input[2][s] * m[2][1];
    buf[1] += input[3][s] * m[3][1];
    buf[1] += input[4][s] * m[4][1];
    output[0][s] = buf[0];
    output[1][s] = buf[1];
  }
}

void Mixer::io_mix53(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[3];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[0] += input[1][s] * m[1][0];
    buf[0] += input[2][s] * m[2][0];
    buf[0] += input[3][s] * m[3][0];
    buf[0] += input[4][s] * m[4][0];
    buf[1]  = input[0][s] * m[0][1];
    buf[1] += input[1][s] * m[1][1];
    buf[1] += input[2][s] * m[2][1];
    buf[1] += input[3][s] * m[3][1];
    buf[1] += input[4][s] * m[4][1];
    buf[2]  = input[0][s] * m[0][2];
    buf[2] += input[1][s] * m[1][2];
    buf[2] += input[2][s] * m[2][2];
    buf[2] += input[3][s] * m[3][2];
    buf[2] += input[4][s] * m[4][2];
    output[0][s] = buf[0];
    output[1][s] = buf[1];
    output[2][s] = buf[2];
  }
}

void Mixer::io_mix54(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[4];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[0] += input[1][s] * m[1][0];
    buf[0] += input[2][s] * m[2][0];
    buf[0] += input[3][s] * m[3][0];
    buf[0] += input[4][s] * m[4][0];
    buf[1]  = input[0][s] * m[0][1];
    buf[1] += input[1][s] * m[1][1];
    buf[1] += input[2][s] * m[2][1];
    buf[1] += input[3][s] * m[3][1];
    buf[1] += input[4][s] * m[4][1];
    buf[2]  = input[0][s] * m[0][2];
    buf[2] += input[1][s] * m[1][2];
    buf[2] += input[2][s] * m[2][2];
    buf[2] += input[3][s] * m[3][2];
    buf[2] += input[4][s] * m[4][2];
    buf[3]  = input[0][s] * m[0][3];
    buf[3] += input[1][s] * m[1][3];
    buf[3] += input[2][s] * m[2][3];
    buf[3] += input[3][s] * m[3][3];
    buf[3] += input[4][s] * m[4][3];
    output[0][s] = buf[0];
    output[1][s] = buf[1];
    output[2][s] = buf[2];
    output[3][s] = buf[3];
  }
}

void Mixer::io_mix55(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[5];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[0] += input[1][s] * m[1][0];
    buf[0] += input[2][s] * m[2][0];
    buf[0] += input[3][s] * m[3][0];
    buf[0] += input[4][s] * m[4][0];
    buf[1]  = input[0][s] * m[0][1];
    buf[1] += input[1][s] * m[1][1];
    buf[1] += input[2][s] * m[2][1];
    buf[1] += input[3][s] * m[3][1];
    buf[1] += input[4][s] * m[4][1];
    buf[2]  = input[0][s] * m[0][2];
    buf[2] += input[1][s] * m[1][2];
    buf[2] += input[2][s] * m[2][2];
    buf[2] += input[3][s] * m[3][2];
    buf[2] += input[4][s] * m[4][2];
    buf[3]  = input[0][s] * m[0][3];
    buf[3] += input[1][s] * m[1][3];
    buf[3] += input[2][s] * m[2][3];
    buf[3] += input[3][s] * m[3][3];
    buf[3] += input[4][s] * m[4][3];
    buf[4]  = input[0][s] * m[0][4];
    buf[4] += input[1][s] * m[1][4];
    buf[4] += input[2][s] * m[2][4];
    buf[4] += input[3][s] * m[3][4];
    buf[4] += input[4][s] * m[4][4];
    output[0][s] = buf[0];
    output[1][s] = buf[1];
    output[2][s] = buf[2];
    output[3][s] = buf[3];
    output[4][s] = buf[4];
  }
}

void Mixer::io_mix56(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[6];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[0] += input[1][s] * m[1][0];
    buf[0] += input[2][s] * m[2][0];
    buf[0] += input[3][s] * m[3][0];
    buf[0] += input[4][s] * m[4][0];
    buf[1]  = input[0][s] * m[0][1];
    buf[1] += input[1][s] * m[1][1];
    buf[1] += input[2][s] * m[2][1];
    buf[1] += input[3][s] * m[3][1];
    buf[1] += input[4][s] * m[4][1];
    buf[2]  = input[0][s] * m[0][2];
    buf[2] += input[1][s] * m[1][2];
    buf[2] += input[2][s] * m[2][2];
    buf[2] += input[3][s] * m[3][2];
    buf[2] += input[4][s] * m[4][2];
    buf[3]  = input[0][s] * m[0][3];
    buf[3] += input[1][s] * m[1][3];
    buf[3] += input[2][s] * m[2][3];
    buf[3] += input[3][s] * m[3][3];
    buf[3] += input[4][s] * m[4][3];
    buf[4]  = input[0][s] * m[0][4];
    buf[4] += input[1][s] * m[1][4];
    buf[4] += input[2][s] * m[2][4];
    buf[4] += input[3][s] * m[3][4];
    buf[4] += input[4][s] * m[4][4];
    buf[5]  = input[0][s] * m[0][5];
    buf[5] += input[1][s] * m[1][5];
    buf[5] += input[2][s] * m[2][5];
    buf[5] += input[3][s] * m[3][5];
    buf[5] += input[4][s] * m[4][5];
    output[0][s] = buf[0];
    output[1][s] = buf[1];
    output[2][s] = buf[2];
    output[3][s] = buf[3];
    output[4][s] = buf[4];
    output[5][s] = buf[5];
  }
}

void Mixer::io_mix61(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[1];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[0] += input[1][s] * m[1][0];
    buf[0] += input[2][s] * m[2][0];
    buf[0] += input[3][s] * m[3][0];
    buf[0] += input[4][s] * m[4][0];
    buf[0] += input[5][s] * m[5][0];
    output[0][s] = buf[0];
  }
}

void Mixer::io_mix62(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[2];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[0] += input[1][s] * m[1][0];
    buf[0] += input[2][s] * m[2][0];
    buf[0] += input[3][s] * m[3][0];
    buf[0] += input[4][s] * m[4][0];
    buf[0] += input[5][s] * m[5][0];
    buf[1]  = input[0][s] * m[0][1];
    buf[1] += input[1][s] * m[1][1];
    buf[1] += input[2][s] * m[2][1];
    buf[1] += input[3][s] * m[3][1];
    buf[1] += input[4][s] * m[4][1];
    buf[1] += input[5][s] * m[5][1];
    output[0][s] = buf[0];
    output[1][s] = buf[1];
  }
}

void Mixer::io_mix63(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[3];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[0] += input[1][s] * m[1][0];
    buf[0] += input[2][s] * m[2][0];
    buf[0] += input[3][s] * m[3][0];
    buf[0] += input[4][s] * m[4][0];
    buf[0] += input[5][s] * m[5][0];
    buf[1]  = input[0][s] * m[0][1];
    buf[1] += input[1][s] * m[1][1];
    buf[1] += input[2][s] * m[2][1];
    buf[1] += input[3][s] * m[3][1];
    buf[1] += input[4][s] * m[4][1];
    buf[1] += input[5][s] * m[5][1];
    buf[2]  = input[0][s] * m[0][2];
    buf[2] += input[1][s] * m[1][2];
    buf[2] += input[2][s] * m[2][2];
    buf[2] += input[3][s] * m[3][2];
    buf[2] += input[4][s] * m[4][2];
    buf[2] += input[5][s] * m[5][2];
    output[0][s] = buf[0];
    output[1][s] = buf[1];
    output[2][s] = buf[2];
  }
}

void Mixer::io_mix64(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[4];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[0] += input[1][s] * m[1][0];
    buf[0] += input[2][s] * m[2][0];
    buf[0] += input[3][s] * m[3][0];
    buf[0] += input[4][s] * m[4][0];
    buf[0] += input[5][s] * m[5][0];
    buf[1]  = input[0][s] * m[0][1];
    buf[1] += input[1][s] * m[1][1];
    buf[1] += input[2][s] * m[2][1];
    buf[1] += input[3][s] * m[3][1];
    buf[1] += input[4][s] * m[4][1];
    buf[1] += input[5][s] * m[5][1];
    buf[2]  = input[0][s] * m[0][2];
    buf[2] += input[1][s] * m[1][2];
    buf[2] += input[2][s] * m[2][2];
    buf[2] += input[3][s] * m[3][2];
    buf[2] += input[4][s] * m[4][2];
    buf[2] += input[5][s] * m[5][2];
    buf[3]  = input[0][s] * m[0][3];
    buf[3] += input[1][s] * m[1][3];
    buf[3] += input[2][s] * m[2][3];
    buf[3] += input[3][s] * m[3][3];
    buf[3] += input[4][s] * m[4][3];
    buf[3] += input[5][s] * m[5][3];
    output[0][s] = buf[0];
    output[1][s] = buf[1];
    output[2][s] = buf[2];
    output[3][s] = buf[3];
  }
}

void Mixer::io_mix65(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[5];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[0] += input[1][s] * m[1][0];
    buf[0] += input[2][s] * m[2][0];
    buf[0] += input[3][s] * m[3][0];
    buf[0] += input[4][s] * m[4][0];
    buf[0] += input[5][s] * m[5][0];
    buf[1]  = input[0][s] * m[0][1];
    buf[1] += input[1][s] * m[1][1];
    buf[1] += input[2][s] * m[2][1];
    buf[1] += input[3][s] * m[3][1];
    buf[1] += input[4][s] * m[4][1];
    buf[1] += input[5][s] * m[5][1];
    buf[2]  = input[0][s] * m[0][2];
    buf[2] += input[1][s] * m[1][2];
    buf[2] += input[2][s] * m[2][2];
    buf[2] += input[3][s] * m[3][2];
    buf[2] += input[4][s] * m[4][2];
    buf[2] += input[5][s] * m[5][2];
    buf[3]  = input[0][s] * m[0][3];
    buf[3] += input[1][s] * m[1][3];
    buf[3] += input[2][s] * m[2][3];
    buf[3] += input[3][s] * m[3][3];
    buf[3] += input[4][s] * m[4][3];
    buf[3] += input[5][s] * m[5][3];
    buf[4]  = input[0][s] * m[0][4];
    buf[4] += input[1][s] * m[1][4];
    buf[4] += input[2][s] * m[2][4];
    buf[4] += input[3][s] * m[3][4];
    buf[4] += input[4][s] * m[4][4];
    buf[4] += input[5][s] * m[5][4];
    output[0][s] = buf[0];
    output[1][s] = buf[1];
    output[2][s] = buf[2];
    output[3][s] = buf[3];
    output[4][s] = buf[4];
  }
}

void Mixer::io_mix66(samples_t input, samples_t output, size_t nsamples)
{
  sample_t buf[6];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = input[0][s] * m[0][0];
    buf[0] += input[1][s] * m[1][0];
    buf[0] += input[2][s] * m[2][0];
    buf[0] += input[3][s] * m[3][0];
    buf[0] += input[4][s] * m[4][0];
    buf[0] += input[5][s] * m[5][0];
    buf[1]  = input[0][s] * m[0][1];
    buf[1] += input[1][s] * m[1][1];
    buf[1] += input[2][s] * m[2][1];
    buf[1] += input[3][s] * m[3][1];
    buf[1] += input[4][s] * m[4][1];
    buf[1] += input[5][s] * m[5][1];
    buf[2]  = input[0][s] * m[0][2];
    buf[2] += input[1][s] * m[1][2];
    buf[2] += input[2][s] * m[2][2];
    buf[2] += input[3][s] * m[3][2];
    buf[2] += input[4][s] * m[4][2];
    buf[2] += input[5][s] * m[5][2];
    buf[3]  = input[0][s] * m[0][3];
    buf[3] += input[1][s] * m[1][3];
    buf[3] += input[2][s] * m[2][3];
    buf[3] += input[3][s] * m[3][3];
    buf[3] += input[4][s] * m[4][3];
    buf[3] += input[5][s] * m[5][3];
    buf[4]  = input[0][s] * m[0][4];
    buf[4] += input[1][s] * m[1][4];
    buf[4] += input[2][s] * m[2][4];
    buf[4] += input[3][s] * m[3][4];
    buf[4] += input[4][s] * m[4][4];
    buf[4] += input[5][s] * m[5][4];
    buf[5]  = input[0][s] * m[0][5];
    buf[5] += input[1][s] * m[1][5];
    buf[5] += input[2][s] * m[2][5];
    buf[5] += input[3][s] * m[3][5];
    buf[5] += input[4][s] * m[4][5];
    buf[5] += input[5][s] * m[5][5];
    output[0][s] = buf[0];
    output[1][s] = buf[1];
    output[2][s] = buf[2];
    output[3][s] = buf[3];
    output[4][s] = buf[4];
    output[5][s] = buf[5];
  }
}

void Mixer::ip_mix11(samples_t samples, size_t nsamples)
{
  sample_t buf[1];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    samples[0][s] = buf[0];
  }
}

void Mixer::ip_mix12(samples_t samples, size_t nsamples)
{
  sample_t buf[2];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[1]  = samples[0][s] * m[0][1];
    samples[0][s] = buf[0];
    samples[1][s] = buf[1];
  }
}

void Mixer::ip_mix13(samples_t samples, size_t nsamples)
{
  sample_t buf[3];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[1]  = samples[0][s] * m[0][1];
    buf[2]  = samples[0][s] * m[0][2];
    samples[0][s] = buf[0];
    samples[1][s] = buf[1];
    samples[2][s] = buf[2];
  }
}

void Mixer::ip_mix14(samples_t samples, size_t nsamples)
{
  sample_t buf[4];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[1]  = samples[0][s] * m[0][1];
    buf[2]  = samples[0][s] * m[0][2];
    buf[3]  = samples[0][s] * m[0][3];
    samples[0][s] = buf[0];
    samples[1][s] = buf[1];
    samples[2][s] = buf[2];
    samples[3][s] = buf[3];
  }
}

void Mixer::ip_mix15(samples_t samples, size_t nsamples)
{
  sample_t buf[5];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[1]  = samples[0][s] * m[0][1];
    buf[2]  = samples[0][s] * m[0][2];
    buf[3]  = samples[0][s] * m[0][3];
    buf[4]  = samples[0][s] * m[0][4];
    samples[0][s] = buf[0];
    samples[1][s] = buf[1];
    samples[2][s] = buf[2];
    samples[3][s] = buf[3];
    samples[4][s] = buf[4];
  }
}

void Mixer::ip_mix16(samples_t samples, size_t nsamples)
{
  sample_t buf[6];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[1]  = samples[0][s] * m[0][1];
    buf[2]  = samples[0][s] * m[0][2];
    buf[3]  = samples[0][s] * m[0][3];
    buf[4]  = samples[0][s] * m[0][4];
    buf[5]  = samples[0][s] * m[0][5];
    samples[0][s] = buf[0];
    samples[1][s] = buf[1];
    samples[2][s] = buf[2];
    samples[3][s] = buf[3];
    samples[4][s] = buf[4];
    samples[5][s] = buf[5];
  }
}

void Mixer::ip_mix21(samples_t samples, size_t nsamples)
{
  sample_t buf[1];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[0] += samples[1][s] * m[1][0];
    samples[0][s] = buf[0];
  }
}

void Mixer::ip_mix22(samples_t samples, size_t nsamples)
{
  sample_t buf[2];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[0] += samples[1][s] * m[1][0];
    buf[1]  = samples[0][s] * m[0][1];
    buf[1] += samples[1][s] * m[1][1];
    samples[0][s] = buf[0];
    samples[1][s] = buf[1];
  }
}

void Mixer::ip_mix23(samples_t samples, size_t nsamples)
{
  sample_t buf[3];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[0] += samples[1][s] * m[1][0];
    buf[1]  = samples[0][s] * m[0][1];
    buf[1] += samples[1][s] * m[1][1];
    buf[2]  = samples[0][s] * m[0][2];
    buf[2] += samples[1][s] * m[1][2];
    samples[0][s] = buf[0];
    samples[1][s] = buf[1];
    samples[2][s] = buf[2];
  }
}

void Mixer::ip_mix24(samples_t samples, size_t nsamples)
{
  sample_t buf[4];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[0] += samples[1][s] * m[1][0];
    buf[1]  = samples[0][s] * m[0][1];
    buf[1] += samples[1][s] * m[1][1];
    buf[2]  = samples[0][s] * m[0][2];
    buf[2] += samples[1][s] * m[1][2];
    buf[3]  = samples[0][s] * m[0][3];
    buf[3] += samples[1][s] * m[1][3];
    samples[0][s] = buf[0];
    samples[1][s] = buf[1];
    samples[2][s] = buf[2];
    samples[3][s] = buf[3];
  }
}

void Mixer::ip_mix25(samples_t samples, size_t nsamples)
{
  sample_t buf[5];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[0] += samples[1][s] * m[1][0];
    buf[1]  = samples[0][s] * m[0][1];
    buf[1] += samples[1][s] * m[1][1];
    buf[2]  = samples[0][s] * m[0][2];
    buf[2] += samples[1][s] * m[1][2];
    buf[3]  = samples[0][s] * m[0][3];
    buf[3] += samples[1][s] * m[1][3];
    buf[4]  = samples[0][s] * m[0][4];
    buf[4] += samples[1][s] * m[1][4];
    samples[0][s] = buf[0];
    samples[1][s] = buf[1];
    samples[2][s] = buf[2];
    samples[3][s] = buf[3];
    samples[4][s] = buf[4];
  }
}

void Mixer::ip_mix26(samples_t samples, size_t nsamples)
{
  sample_t buf[6];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[0] += samples[1][s] * m[1][0];
    buf[1]  = samples[0][s] * m[0][1];
    buf[1] += samples[1][s] * m[1][1];
    buf[2]  = samples[0][s] * m[0][2];
    buf[2] += samples[1][s] * m[1][2];
    buf[3]  = samples[0][s] * m[0][3];
    buf[3] += samples[1][s] * m[1][3];
    buf[4]  = samples[0][s] * m[0][4];
    buf[4] += samples[1][s] * m[1][4];
    buf[5]  = samples[0][s] * m[0][5];
    buf[5] += samples[1][s] * m[1][5];
    samples[0][s] = buf[0];
    samples[1][s] = buf[1];
    samples[2][s] = buf[2];
    samples[3][s] = buf[3];
    samples[4][s] = buf[4];
    samples[5][s] = buf[5];
  }
}

void Mixer::ip_mix31(samples_t samples, size_t nsamples)
{
  sample_t buf[1];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[0] += samples[1][s] * m[1][0];
    buf[0] += samples[2][s] * m[2][0];
    samples[0][s] = buf[0];
  }
}

void Mixer::ip_mix32(samples_t samples, size_t nsamples)
{
  sample_t buf[2];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[0] += samples[1][s] * m[1][0];
    buf[0] += samples[2][s] * m[2][0];
    buf[1]  = samples[0][s] * m[0][1];
    buf[1] += samples[1][s] * m[1][1];
    buf[1] += samples[2][s] * m[2][1];
    samples[0][s] = buf[0];
    samples[1][s] = buf[1];
  }
}

void Mixer::ip_mix33(samples_t samples, size_t nsamples)
{
  sample_t buf[3];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[0] += samples[1][s] * m[1][0];
    buf[0] += samples[2][s] * m[2][0];
    buf[1]  = samples[0][s] * m[0][1];
    buf[1] += samples[1][s] * m[1][1];
    buf[1] += samples[2][s] * m[2][1];
    buf[2]  = samples[0][s] * m[0][2];
    buf[2] += samples[1][s] * m[1][2];
    buf[2] += samples[2][s] * m[2][2];
    samples[0][s] = buf[0];
    samples[1][s] = buf[1];
    samples[2][s] = buf[2];
  }
}

void Mixer::ip_mix34(samples_t samples, size_t nsamples)
{
  sample_t buf[4];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[0] += samples[1][s] * m[1][0];
    buf[0] += samples[2][s] * m[2][0];
    buf[1]  = samples[0][s] * m[0][1];
    buf[1] += samples[1][s] * m[1][1];
    buf[1] += samples[2][s] * m[2][1];
    buf[2]  = samples[0][s] * m[0][2];
    buf[2] += samples[1][s] * m[1][2];
    buf[2] += samples[2][s] * m[2][2];
    buf[3]  = samples[0][s] * m[0][3];
    buf[3] += samples[1][s] * m[1][3];
    buf[3] += samples[2][s] * m[2][3];
    samples[0][s] = buf[0];
    samples[1][s] = buf[1];
    samples[2][s] = buf[2];
    samples[3][s] = buf[3];
  }
}

void Mixer::ip_mix35(samples_t samples, size_t nsamples)
{
  sample_t buf[5];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[0] += samples[1][s] * m[1][0];
    buf[0] += samples[2][s] * m[2][0];
    buf[1]  = samples[0][s] * m[0][1];
    buf[1] += samples[1][s] * m[1][1];
    buf[1] += samples[2][s] * m[2][1];
    buf[2]  = samples[0][s] * m[0][2];
    buf[2] += samples[1][s] * m[1][2];
    buf[2] += samples[2][s] * m[2][2];
    buf[3]  = samples[0][s] * m[0][3];
    buf[3] += samples[1][s] * m[1][3];
    buf[3] += samples[2][s] * m[2][3];
    buf[4]  = samples[0][s] * m[0][4];
    buf[4] += samples[1][s] * m[1][4];
    buf[4] += samples[2][s] * m[2][4];
    samples[0][s] = buf[0];
    samples[1][s] = buf[1];
    samples[2][s] = buf[2];
    samples[3][s] = buf[3];
    samples[4][s] = buf[4];
  }
}

void Mixer::ip_mix36(samples_t samples, size_t nsamples)
{
  sample_t buf[6];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[0] += samples[1][s] * m[1][0];
    buf[0] += samples[2][s] * m[2][0];
    buf[1]  = samples[0][s] * m[0][1];
    buf[1] += samples[1][s] * m[1][1];
    buf[1] += samples[2][s] * m[2][1];
    buf[2]  = samples[0][s] * m[0][2];
    buf[2] += samples[1][s] * m[1][2];
    buf[2] += samples[2][s] * m[2][2];
    buf[3]  = samples[0][s] * m[0][3];
    buf[3] += samples[1][s] * m[1][3];
    buf[3] += samples[2][s] * m[2][3];
    buf[4]  = samples[0][s] * m[0][4];
    buf[4] += samples[1][s] * m[1][4];
    buf[4] += samples[2][s] * m[2][4];
    buf[5]  = samples[0][s] * m[0][5];
    buf[5] += samples[1][s] * m[1][5];
    buf[5] += samples[2][s] * m[2][5];
    samples[0][s] = buf[0];
    samples[1][s] = buf[1];
    samples[2][s] = buf[2];
    samples[3][s] = buf[3];
    samples[4][s] = buf[4];
    samples[5][s] = buf[5];
  }
}

void Mixer::ip_mix41(samples_t samples, size_t nsamples)
{
  sample_t buf[1];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[0] += samples[1][s] * m[1][0];
    buf[0] += samples[2][s] * m[2][0];
    buf[0] += samples[3][s] * m[3][0];
    samples[0][s] = buf[0];
  }
}

void Mixer::ip_mix42(samples_t samples, size_t nsamples)
{
  sample_t buf[2];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[0] += samples[1][s] * m[1][0];
    buf[0] += samples[2][s] * m[2][0];
    buf[0] += samples[3][s] * m[3][0];
    buf[1]  = samples[0][s] * m[0][1];
    buf[1] += samples[1][s] * m[1][1];
    buf[1] += samples[2][s] * m[2][1];
    buf[1] += samples[3][s] * m[3][1];
    samples[0][s] = buf[0];
    samples[1][s] = buf[1];
  }
}

void Mixer::ip_mix43(samples_t samples, size_t nsamples)
{
  sample_t buf[3];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[0] += samples[1][s] * m[1][0];
    buf[0] += samples[2][s] * m[2][0];
    buf[0] += samples[3][s] * m[3][0];
    buf[1]  = samples[0][s] * m[0][1];
    buf[1] += samples[1][s] * m[1][1];
    buf[1] += samples[2][s] * m[2][1];
    buf[1] += samples[3][s] * m[3][1];
    buf[2]  = samples[0][s] * m[0][2];
    buf[2] += samples[1][s] * m[1][2];
    buf[2] += samples[2][s] * m[2][2];
    buf[2] += samples[3][s] * m[3][2];
    samples[0][s] = buf[0];
    samples[1][s] = buf[1];
    samples[2][s] = buf[2];
  }
}

void Mixer::ip_mix44(samples_t samples, size_t nsamples)
{
  sample_t buf[4];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[0] += samples[1][s] * m[1][0];
    buf[0] += samples[2][s] * m[2][0];
    buf[0] += samples[3][s] * m[3][0];
    buf[1]  = samples[0][s] * m[0][1];
    buf[1] += samples[1][s] * m[1][1];
    buf[1] += samples[2][s] * m[2][1];
    buf[1] += samples[3][s] * m[3][1];
    buf[2]  = samples[0][s] * m[0][2];
    buf[2] += samples[1][s] * m[1][2];
    buf[2] += samples[2][s] * m[2][2];
    buf[2] += samples[3][s] * m[3][2];
    buf[3]  = samples[0][s] * m[0][3];
    buf[3] += samples[1][s] * m[1][3];
    buf[3] += samples[2][s] * m[2][3];
    buf[3] += samples[3][s] * m[3][3];
    samples[0][s] = buf[0];
    samples[1][s] = buf[1];
    samples[2][s] = buf[2];
    samples[3][s] = buf[3];
  }
}

void Mixer::ip_mix45(samples_t samples, size_t nsamples)
{
  sample_t buf[5];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[0] += samples[1][s] * m[1][0];
    buf[0] += samples[2][s] * m[2][0];
    buf[0] += samples[3][s] * m[3][0];
    buf[1]  = samples[0][s] * m[0][1];
    buf[1] += samples[1][s] * m[1][1];
    buf[1] += samples[2][s] * m[2][1];
    buf[1] += samples[3][s] * m[3][1];
    buf[2]  = samples[0][s] * m[0][2];
    buf[2] += samples[1][s] * m[1][2];
    buf[2] += samples[2][s] * m[2][2];
    buf[2] += samples[3][s] * m[3][2];
    buf[3]  = samples[0][s] * m[0][3];
    buf[3] += samples[1][s] * m[1][3];
    buf[3] += samples[2][s] * m[2][3];
    buf[3] += samples[3][s] * m[3][3];
    buf[4]  = samples[0][s] * m[0][4];
    buf[4] += samples[1][s] * m[1][4];
    buf[4] += samples[2][s] * m[2][4];
    buf[4] += samples[3][s] * m[3][4];
    samples[0][s] = buf[0];
    samples[1][s] = buf[1];
    samples[2][s] = buf[2];
    samples[3][s] = buf[3];
    samples[4][s] = buf[4];
  }
}

void Mixer::ip_mix46(samples_t samples, size_t nsamples)
{
  sample_t buf[6];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[0] += samples[1][s] * m[1][0];
    buf[0] += samples[2][s] * m[2][0];
    buf[0] += samples[3][s] * m[3][0];
    buf[1]  = samples[0][s] * m[0][1];
    buf[1] += samples[1][s] * m[1][1];
    buf[1] += samples[2][s] * m[2][1];
    buf[1] += samples[3][s] * m[3][1];
    buf[2]  = samples[0][s] * m[0][2];
    buf[2] += samples[1][s] * m[1][2];
    buf[2] += samples[2][s] * m[2][2];
    buf[2] += samples[3][s] * m[3][2];
    buf[3]  = samples[0][s] * m[0][3];
    buf[3] += samples[1][s] * m[1][3];
    buf[3] += samples[2][s] * m[2][3];
    buf[3] += samples[3][s] * m[3][3];
    buf[4]  = samples[0][s] * m[0][4];
    buf[4] += samples[1][s] * m[1][4];
    buf[4] += samples[2][s] * m[2][4];
    buf[4] += samples[3][s] * m[3][4];
    buf[5]  = samples[0][s] * m[0][5];
    buf[5] += samples[1][s] * m[1][5];
    buf[5] += samples[2][s] * m[2][5];
    buf[5] += samples[3][s] * m[3][5];
    samples[0][s] = buf[0];
    samples[1][s] = buf[1];
    samples[2][s] = buf[2];
    samples[3][s] = buf[3];
    samples[4][s] = buf[4];
    samples[5][s] = buf[5];
  }
}

void Mixer::ip_mix51(samples_t samples, size_t nsamples)
{
  sample_t buf[1];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[0] += samples[1][s] * m[1][0];
    buf[0] += samples[2][s] * m[2][0];
    buf[0] += samples[3][s] * m[3][0];
    buf[0] += samples[4][s] * m[4][0];
    samples[0][s] = buf[0];
  }
}

void Mixer::ip_mix52(samples_t samples, size_t nsamples)
{
  sample_t buf[2];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[0] += samples[1][s] * m[1][0];
    buf[0] += samples[2][s] * m[2][0];
    buf[0] += samples[3][s] * m[3][0];
    buf[0] += samples[4][s] * m[4][0];
    buf[1]  = samples[0][s] * m[0][1];
    buf[1] += samples[1][s] * m[1][1];
    buf[1] += samples[2][s] * m[2][1];
    buf[1] += samples[3][s] * m[3][1];
    buf[1] += samples[4][s] * m[4][1];
    samples[0][s] = buf[0];
    samples[1][s] = buf[1];
  }
}

void Mixer::ip_mix53(samples_t samples, size_t nsamples)
{
  sample_t buf[3];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[0] += samples[1][s] * m[1][0];
    buf[0] += samples[2][s] * m[2][0];
    buf[0] += samples[3][s] * m[3][0];
    buf[0] += samples[4][s] * m[4][0];
    buf[1]  = samples[0][s] * m[0][1];
    buf[1] += samples[1][s] * m[1][1];
    buf[1] += samples[2][s] * m[2][1];
    buf[1] += samples[3][s] * m[3][1];
    buf[1] += samples[4][s] * m[4][1];
    buf[2]  = samples[0][s] * m[0][2];
    buf[2] += samples[1][s] * m[1][2];
    buf[2] += samples[2][s] * m[2][2];
    buf[2] += samples[3][s] * m[3][2];
    buf[2] += samples[4][s] * m[4][2];
    samples[0][s] = buf[0];
    samples[1][s] = buf[1];
    samples[2][s] = buf[2];
  }
}

void Mixer::ip_mix54(samples_t samples, size_t nsamples)
{
  sample_t buf[4];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[0] += samples[1][s] * m[1][0];
    buf[0] += samples[2][s] * m[2][0];
    buf[0] += samples[3][s] * m[3][0];
    buf[0] += samples[4][s] * m[4][0];
    buf[1]  = samples[0][s] * m[0][1];
    buf[1] += samples[1][s] * m[1][1];
    buf[1] += samples[2][s] * m[2][1];
    buf[1] += samples[3][s] * m[3][1];
    buf[1] += samples[4][s] * m[4][1];
    buf[2]  = samples[0][s] * m[0][2];
    buf[2] += samples[1][s] * m[1][2];
    buf[2] += samples[2][s] * m[2][2];
    buf[2] += samples[3][s] * m[3][2];
    buf[2] += samples[4][s] * m[4][2];
    buf[3]  = samples[0][s] * m[0][3];
    buf[3] += samples[1][s] * m[1][3];
    buf[3] += samples[2][s] * m[2][3];
    buf[3] += samples[3][s] * m[3][3];
    buf[3] += samples[4][s] * m[4][3];
    samples[0][s] = buf[0];
    samples[1][s] = buf[1];
    samples[2][s] = buf[2];
    samples[3][s] = buf[3];
  }
}

void Mixer::ip_mix55(samples_t samples, size_t nsamples)
{
  sample_t buf[5];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[0] += samples[1][s] * m[1][0];
    buf[0] += samples[2][s] * m[2][0];
    buf[0] += samples[3][s] * m[3][0];
    buf[0] += samples[4][s] * m[4][0];
    buf[1]  = samples[0][s] * m[0][1];
    buf[1] += samples[1][s] * m[1][1];
    buf[1] += samples[2][s] * m[2][1];
    buf[1] += samples[3][s] * m[3][1];
    buf[1] += samples[4][s] * m[4][1];
    buf[2]  = samples[0][s] * m[0][2];
    buf[2] += samples[1][s] * m[1][2];
    buf[2] += samples[2][s] * m[2][2];
    buf[2] += samples[3][s] * m[3][2];
    buf[2] += samples[4][s] * m[4][2];
    buf[3]  = samples[0][s] * m[0][3];
    buf[3] += samples[1][s] * m[1][3];
    buf[3] += samples[2][s] * m[2][3];
    buf[3] += samples[3][s] * m[3][3];
    buf[3] += samples[4][s] * m[4][3];
    buf[4]  = samples[0][s] * m[0][4];
    buf[4] += samples[1][s] * m[1][4];
    buf[4] += samples[2][s] * m[2][4];
    buf[4] += samples[3][s] * m[3][4];
    buf[4] += samples[4][s] * m[4][4];
    samples[0][s] = buf[0];
    samples[1][s] = buf[1];
    samples[2][s] = buf[2];
    samples[3][s] = buf[3];
    samples[4][s] = buf[4];
  }
}

void Mixer::ip_mix56(samples_t samples, size_t nsamples)
{
  sample_t buf[6];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[0] += samples[1][s] * m[1][0];
    buf[0] += samples[2][s] * m[2][0];
    buf[0] += samples[3][s] * m[3][0];
    buf[0] += samples[4][s] * m[4][0];
    buf[1]  = samples[0][s] * m[0][1];
    buf[1] += samples[1][s] * m[1][1];
    buf[1] += samples[2][s] * m[2][1];
    buf[1] += samples[3][s] * m[3][1];
    buf[1] += samples[4][s] * m[4][1];
    buf[2]  = samples[0][s] * m[0][2];
    buf[2] += samples[1][s] * m[1][2];
    buf[2] += samples[2][s] * m[2][2];
    buf[2] += samples[3][s] * m[3][2];
    buf[2] += samples[4][s] * m[4][2];
    buf[3]  = samples[0][s] * m[0][3];
    buf[3] += samples[1][s] * m[1][3];
    buf[3] += samples[2][s] * m[2][3];
    buf[3] += samples[3][s] * m[3][3];
    buf[3] += samples[4][s] * m[4][3];
    buf[4]  = samples[0][s] * m[0][4];
    buf[4] += samples[1][s] * m[1][4];
    buf[4] += samples[2][s] * m[2][4];
    buf[4] += samples[3][s] * m[3][4];
    buf[4] += samples[4][s] * m[4][4];
    buf[5]  = samples[0][s] * m[0][5];
    buf[5] += samples[1][s] * m[1][5];
    buf[5] += samples[2][s] * m[2][5];
    buf[5] += samples[3][s] * m[3][5];
    buf[5] += samples[4][s] * m[4][5];
    samples[0][s] = buf[0];
    samples[1][s] = buf[1];
    samples[2][s] = buf[2];
    samples[3][s] = buf[3];
    samples[4][s] = buf[4];
    samples[5][s] = buf[5];
  }
}

void Mixer::ip_mix61(samples_t samples, size_t nsamples)
{
  sample_t buf[1];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[0] += samples[1][s] * m[1][0];
    buf[0] += samples[2][s] * m[2][0];
    buf[0] += samples[3][s] * m[3][0];
    buf[0] += samples[4][s] * m[4][0];
    buf[0] += samples[5][s] * m[5][0];
    samples[0][s] = buf[0];
  }
}

void Mixer::ip_mix62(samples_t samples, size_t nsamples)
{
  sample_t buf[2];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[0] += samples[1][s] * m[1][0];
    buf[0] += samples[2][s] * m[2][0];
    buf[0] += samples[3][s] * m[3][0];
    buf[0] += samples[4][s] * m[4][0];
    buf[0] += samples[5][s] * m[5][0];
    buf[1]  = samples[0][s] * m[0][1];
    buf[1] += samples[1][s] * m[1][1];
    buf[1] += samples[2][s] * m[2][1];
    buf[1] += samples[3][s] * m[3][1];
    buf[1] += samples[4][s] * m[4][1];
    buf[1] += samples[5][s] * m[5][1];
    samples[0][s] = buf[0];
    samples[1][s] = buf[1];
  }
}

void Mixer::ip_mix63(samples_t samples, size_t nsamples)
{
  sample_t buf[3];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[0] += samples[1][s] * m[1][0];
    buf[0] += samples[2][s] * m[2][0];
    buf[0] += samples[3][s] * m[3][0];
    buf[0] += samples[4][s] * m[4][0];
    buf[0] += samples[5][s] * m[5][0];
    buf[1]  = samples[0][s] * m[0][1];
    buf[1] += samples[1][s] * m[1][1];
    buf[1] += samples[2][s] * m[2][1];
    buf[1] += samples[3][s] * m[3][1];
    buf[1] += samples[4][s] * m[4][1];
    buf[1] += samples[5][s] * m[5][1];
    buf[2]  = samples[0][s] * m[0][2];
    buf[2] += samples[1][s] * m[1][2];
    buf[2] += samples[2][s] * m[2][2];
    buf[2] += samples[3][s] * m[3][2];
    buf[2] += samples[4][s] * m[4][2];
    buf[2] += samples[5][s] * m[5][2];
    samples[0][s] = buf[0];
    samples[1][s] = buf[1];
    samples[2][s] = buf[2];
  }
}

void Mixer::ip_mix64(samples_t samples, size_t nsamples)
{
  sample_t buf[4];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[0] += samples[1][s] * m[1][0];
    buf[0] += samples[2][s] * m[2][0];
    buf[0] += samples[3][s] * m[3][0];
    buf[0] += samples[4][s] * m[4][0];
    buf[0] += samples[5][s] * m[5][0];
    buf[1]  = samples[0][s] * m[0][1];
    buf[1] += samples[1][s] * m[1][1];
    buf[1] += samples[2][s] * m[2][1];
    buf[1] += samples[3][s] * m[3][1];
    buf[1] += samples[4][s] * m[4][1];
    buf[1] += samples[5][s] * m[5][1];
    buf[2]  = samples[0][s] * m[0][2];
    buf[2] += samples[1][s] * m[1][2];
    buf[2] += samples[2][s] * m[2][2];
    buf[2] += samples[3][s] * m[3][2];
    buf[2] += samples[4][s] * m[4][2];
    buf[2] += samples[5][s] * m[5][2];
    buf[3]  = samples[0][s] * m[0][3];
    buf[3] += samples[1][s] * m[1][3];
    buf[3] += samples[2][s] * m[2][3];
    buf[3] += samples[3][s] * m[3][3];
    buf[3] += samples[4][s] * m[4][3];
    buf[3] += samples[5][s] * m[5][3];
    samples[0][s] = buf[0];
    samples[1][s] = buf[1];
    samples[2][s] = buf[2];
    samples[3][s] = buf[3];
  }
}

void Mixer::ip_mix65(samples_t samples, size_t nsamples)
{
  sample_t buf[5];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[0] += samples[1][s] * m[1][0];
    buf[0] += samples[2][s] * m[2][0];
    buf[0] += samples[3][s] * m[3][0];
    buf[0] += samples[4][s] * m[4][0];
    buf[0] += samples[5][s] * m[5][0];
    buf[1]  = samples[0][s] * m[0][1];
    buf[1] += samples[1][s] * m[1][1];
    buf[1] += samples[2][s] * m[2][1];
    buf[1] += samples[3][s] * m[3][1];
    buf[1] += samples[4][s] * m[4][1];
    buf[1] += samples[5][s] * m[5][1];
    buf[2]  = samples[0][s] * m[0][2];
    buf[2] += samples[1][s] * m[1][2];
    buf[2] += samples[2][s] * m[2][2];
    buf[2] += samples[3][s] * m[3][2];
    buf[2] += samples[4][s] * m[4][2];
    buf[2] += samples[5][s] * m[5][2];
    buf[3]  = samples[0][s] * m[0][3];
    buf[3] += samples[1][s] * m[1][3];
    buf[3] += samples[2][s] * m[2][3];
    buf[3] += samples[3][s] * m[3][3];
    buf[3] += samples[4][s] * m[4][3];
    buf[3] += samples[5][s] * m[5][3];
    buf[4]  = samples[0][s] * m[0][4];
    buf[4] += samples[1][s] * m[1][4];
    buf[4] += samples[2][s] * m[2][4];
    buf[4] += samples[3][s] * m[3][4];
    buf[4] += samples[4][s] * m[4][4];
    buf[4] += samples[5][s] * m[5][4];
    samples[0][s] = buf[0];
    samples[1][s] = buf[1];
    samples[2][s] = buf[2];
    samples[3][s] = buf[3];
    samples[4][s] = buf[4];
  }
}

void Mixer::ip_mix66(samples_t samples, size_t nsamples)
{
  sample_t buf[6];
  for (size_t s = 0; s < nsamples; s++)
  {
    buf[0]  = samples[0][s] * m[0][0];
    buf[0] += samples[1][s] * m[1][0];
    buf[0] += samples[2][s] * m[2][0];
    buf[0] += samples[3][s] * m[3][0];
    buf[0] += samples[4][s] * m[4][0];
    buf[0] += samples[5][s] * m[5][0];
    buf[1]  = samples[0][s] * m[0][1];
    buf[1] += samples[1][s] * m[1][1];
    buf[1] += samples[2][s] * m[2][1];
    buf[1] += samples[3][s] * m[3][1];
    buf[1] += samples[4][s] * m[4][1];
    buf[1] += samples[5][s] * m[5][1];
    buf[2]  = samples[0][s] * m[0][2];
    buf[2] += samples[1][s] * m[1][2];
    buf[2] += samples[2][s] * m[2][2];
    buf[2] += samples[3][s] * m[3][2];
    buf[2] += samples[4][s] * m[4][2];
    buf[2] += samples[5][s] * m[5][2];
    buf[3]  = samples[0][s] * m[0][3];
    buf[3] += samples[1][s] * m[1][3];
    buf[3] += samples[2][s] * m[2][3];
    buf[3] += samples[3][s] * m[3][3];
    buf[3] += samples[4][s] * m[4][3];
    buf[3] += samples[5][s] * m[5][3];
    buf[4]  = samples[0][s] * m[0][4];
    buf[4] += samples[1][s] * m[1][4];
    buf[4] += samples[2][s] * m[2][4];
    buf[4] += samples[3][s] * m[3][4];
    buf[4] += samples[4][s] * m[4][4];
    buf[4] += samples[5][s] * m[5][4];
    buf[5]  = samples[0][s] * m[0][5];
    buf[5] += samples[1][s] * m[1][5];
    buf[5] += samples[2][s] * m[2][5];
    buf[5] += samples[3][s] * m[3][5];
    buf[5] += samples[4][s] * m[4][5];
    buf[5] += samples[5][s] * m[5][5];
    samples[0][s] = buf[0];
    samples[1][s] = buf[1];
    samples[2][s] = buf[2];
    samples[3][s] = buf[3];
    samples[4][s] = buf[4];
    samples[5][s] = buf[5];
  }
}

