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

typedef void (Mixer::*io_mixfunc_t)(samples_t, samples_t, size_t); // input-output mixing

static const io_mixfunc_t io_mix_tbl[NCHANNELS][NCHANNELS] = {
  { &Mixer::io_mix11, &Mixer::io_mix12, &Mixer::io_mix13, &Mixer::io_mix14, &Mixer::io_mix15, &Mixer::io_mix16 },
  { &Mixer::io_mix21, &Mixer::io_mix22, &Mixer::io_mix23, &Mixer::io_mix24, &Mixer::io_mix25, &Mixer::io_mix26 },
  { &Mixer::io_mix31, &Mixer::io_mix32, &Mixer::io_mix33, &Mixer::io_mix34, &Mixer::io_mix35, &Mixer::io_mix36 },
  { &Mixer::io_mix41, &Mixer::io_mix42, &Mixer::io_mix43, &Mixer::io_mix44, &Mixer::io_mix45, &Mixer::io_mix46 },
  { &Mixer::io_mix51, &Mixer::io_mix52, &Mixer::io_mix53, &Mixer::io_mix54, &Mixer::io_mix55, &Mixer::io_mix56 },
  { &Mixer::io_mix61, &Mixer::io_mix62, &Mixer::io_mix63, &Mixer::io_mix64, &Mixer::io_mix65, &Mixer::io_mix66 },
};

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

