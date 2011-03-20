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

typedef void (Mixer::*ip_mixfunc_t)(samples_t, size_t); // in-place mixing

static const ip_mixfunc_t ip_mix_tbl[NCHANNELS][NCHANNELS] = {
  { &Mixer::ip_mix11, &Mixer::ip_mix12, &Mixer::ip_mix13, &Mixer::ip_mix14, &Mixer::ip_mix15, &Mixer::ip_mix16 },
  { &Mixer::ip_mix21, &Mixer::ip_mix22, &Mixer::ip_mix23, &Mixer::ip_mix24, &Mixer::ip_mix25, &Mixer::ip_mix26 },
  { &Mixer::ip_mix31, &Mixer::ip_mix32, &Mixer::ip_mix33, &Mixer::ip_mix34, &Mixer::ip_mix35, &Mixer::ip_mix36 },
  { &Mixer::ip_mix41, &Mixer::ip_mix42, &Mixer::ip_mix43, &Mixer::ip_mix44, &Mixer::ip_mix45, &Mixer::ip_mix46 },
  { &Mixer::ip_mix51, &Mixer::ip_mix52, &Mixer::ip_mix53, &Mixer::ip_mix54, &Mixer::ip_mix55, &Mixer::ip_mix56 },
  { &Mixer::ip_mix61, &Mixer::ip_mix62, &Mixer::ip_mix63, &Mixer::ip_mix64, &Mixer::ip_mix65, &Mixer::ip_mix66 },
};

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

