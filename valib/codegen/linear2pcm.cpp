typedef void (Converter::*convert_t)(uint8_t *rawdata, samples_t samples, size_t size);
static const int linear2pcm_formats[] = { FORMAT_PCM16, FORMAT_PCM24, FORMAT_PCM32, FORMAT_PCM16_BE, FORMAT_PCM24_BE, FORMAT_PCM32_BE, FORMAT_PCMFLOAT, FORMAT_PCMDOUBLE };

static const convert_t pcm2linear_tbl[NCHANNELS][8] = {
 { pcm16_linear_1ch, pcm24_linear_1ch, pcm32_linear_1ch, pcm16_be_linear_1ch, pcm24_be_linear_1ch, pcm32_be_linear_1ch, pcmfloat_linear_1ch, pcmdouble_linear_1ch },
 { pcm16_linear_2ch, pcm24_linear_2ch, pcm32_linear_2ch, pcm16_be_linear_2ch, pcm24_be_linear_2ch, pcm32_be_linear_2ch, pcmfloat_linear_2ch, pcmdouble_linear_2ch },
 { pcm16_linear_3ch, pcm24_linear_3ch, pcm32_linear_3ch, pcm16_be_linear_3ch, pcm24_be_linear_3ch, pcm32_be_linear_3ch, pcmfloat_linear_3ch, pcmdouble_linear_3ch },
 { pcm16_linear_4ch, pcm24_linear_4ch, pcm32_linear_4ch, pcm16_be_linear_4ch, pcm24_be_linear_4ch, pcm32_be_linear_4ch, pcmfloat_linear_4ch, pcmdouble_linear_4ch },
 { pcm16_linear_5ch, pcm24_linear_5ch, pcm32_linear_5ch, pcm16_be_linear_5ch, pcm24_be_linear_5ch, pcm32_be_linear_5ch, pcmfloat_linear_5ch, pcmdouble_linear_5ch },
 { pcm16_linear_6ch, pcm24_linear_6ch, pcm32_linear_6ch, pcm16_be_linear_6ch, pcm24_be_linear_6ch, pcm32_be_linear_6ch, pcmfloat_linear_6ch, pcmdouble_linear_6ch },
};

void linear_pcm16_1ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int16_t *dst = (int16_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2le16(s2i(*src[0])); src[0]++;
    dst += 1;
  }
  restore_rounding(r);
}
void linear_pcm24_1ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int24_t *dst = (int24_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2le24(s2i(*src[0])); src[0]++;
    dst += 1;
  }
  restore_rounding(r);
}
void linear_pcm32_1ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int32_t *dst = (int32_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2le32(s2i(*src[0])); src[0]++;
    dst += 1;
  }
  restore_rounding(r);
}
void linear_pcm16_be_1ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int16_t *dst = (int16_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2be16(s2i(*src[0])); src[0]++;
    dst += 1;
  }
  restore_rounding(r);
}
void linear_pcm24_be_1ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int24_t *dst = (int24_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2be24(s2i(*src[0])); src[0]++;
    dst += 1;
  }
  restore_rounding(r);
}
void linear_pcm32_be_1ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int32_t *dst = (int32_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2be32(s2i(*src[0])); src[0]++;
    dst += 1;
  }
  restore_rounding(r);
}
void linear_pcmfloat_1ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  float *dst = (float *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = float(*src[0]); src[0]++;
    dst += 1;
  }
  restore_rounding(r);
}
void linear_pcmdouble_1ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  double *dst = (double *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = double(*src[0]); src[0]++;
    dst += 1;
  }
  restore_rounding(r);
}
void linear_pcm16_2ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int16_t *dst = (int16_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2le16(s2i(*src[0])); src[0]++;
    dst[1] = int2le16(s2i(*src[1])); src[1]++;
    dst += 2;
  }
  restore_rounding(r);
}
void linear_pcm24_2ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int24_t *dst = (int24_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2le24(s2i(*src[0])); src[0]++;
    dst[1] = int2le24(s2i(*src[1])); src[1]++;
    dst += 2;
  }
  restore_rounding(r);
}
void linear_pcm32_2ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int32_t *dst = (int32_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2le32(s2i(*src[0])); src[0]++;
    dst[1] = int2le32(s2i(*src[1])); src[1]++;
    dst += 2;
  }
  restore_rounding(r);
}
void linear_pcm16_be_2ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int16_t *dst = (int16_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2be16(s2i(*src[0])); src[0]++;
    dst[1] = int2be16(s2i(*src[1])); src[1]++;
    dst += 2;
  }
  restore_rounding(r);
}
void linear_pcm24_be_2ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int24_t *dst = (int24_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2be24(s2i(*src[0])); src[0]++;
    dst[1] = int2be24(s2i(*src[1])); src[1]++;
    dst += 2;
  }
  restore_rounding(r);
}
void linear_pcm32_be_2ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int32_t *dst = (int32_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2be32(s2i(*src[0])); src[0]++;
    dst[1] = int2be32(s2i(*src[1])); src[1]++;
    dst += 2;
  }
  restore_rounding(r);
}
void linear_pcmfloat_2ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  float *dst = (float *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = float(*src[0]); src[0]++;
    dst[1] = float(*src[1]); src[1]++;
    dst += 2;
  }
  restore_rounding(r);
}
void linear_pcmdouble_2ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  double *dst = (double *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = double(*src[0]); src[0]++;
    dst[1] = double(*src[1]); src[1]++;
    dst += 2;
  }
  restore_rounding(r);
}
void linear_pcm16_3ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int16_t *dst = (int16_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2le16(s2i(*src[0])); src[0]++;
    dst[1] = int2le16(s2i(*src[1])); src[1]++;
    dst[2] = int2le16(s2i(*src[2])); src[2]++;
    dst += 3;
  }
  restore_rounding(r);
}
void linear_pcm24_3ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int24_t *dst = (int24_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2le24(s2i(*src[0])); src[0]++;
    dst[1] = int2le24(s2i(*src[1])); src[1]++;
    dst[2] = int2le24(s2i(*src[2])); src[2]++;
    dst += 3;
  }
  restore_rounding(r);
}
void linear_pcm32_3ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int32_t *dst = (int32_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2le32(s2i(*src[0])); src[0]++;
    dst[1] = int2le32(s2i(*src[1])); src[1]++;
    dst[2] = int2le32(s2i(*src[2])); src[2]++;
    dst += 3;
  }
  restore_rounding(r);
}
void linear_pcm16_be_3ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int16_t *dst = (int16_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2be16(s2i(*src[0])); src[0]++;
    dst[1] = int2be16(s2i(*src[1])); src[1]++;
    dst[2] = int2be16(s2i(*src[2])); src[2]++;
    dst += 3;
  }
  restore_rounding(r);
}
void linear_pcm24_be_3ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int24_t *dst = (int24_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2be24(s2i(*src[0])); src[0]++;
    dst[1] = int2be24(s2i(*src[1])); src[1]++;
    dst[2] = int2be24(s2i(*src[2])); src[2]++;
    dst += 3;
  }
  restore_rounding(r);
}
void linear_pcm32_be_3ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int32_t *dst = (int32_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2be32(s2i(*src[0])); src[0]++;
    dst[1] = int2be32(s2i(*src[1])); src[1]++;
    dst[2] = int2be32(s2i(*src[2])); src[2]++;
    dst += 3;
  }
  restore_rounding(r);
}
void linear_pcmfloat_3ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  float *dst = (float *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = float(*src[0]); src[0]++;
    dst[1] = float(*src[1]); src[1]++;
    dst[2] = float(*src[2]); src[2]++;
    dst += 3;
  }
  restore_rounding(r);
}
void linear_pcmdouble_3ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  double *dst = (double *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = double(*src[0]); src[0]++;
    dst[1] = double(*src[1]); src[1]++;
    dst[2] = double(*src[2]); src[2]++;
    dst += 3;
  }
  restore_rounding(r);
}
void linear_pcm16_4ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int16_t *dst = (int16_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2le16(s2i(*src[0])); src[0]++;
    dst[1] = int2le16(s2i(*src[1])); src[1]++;
    dst[2] = int2le16(s2i(*src[2])); src[2]++;
    dst[3] = int2le16(s2i(*src[3])); src[3]++;
    dst += 4;
  }
  restore_rounding(r);
}
void linear_pcm24_4ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int24_t *dst = (int24_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2le24(s2i(*src[0])); src[0]++;
    dst[1] = int2le24(s2i(*src[1])); src[1]++;
    dst[2] = int2le24(s2i(*src[2])); src[2]++;
    dst[3] = int2le24(s2i(*src[3])); src[3]++;
    dst += 4;
  }
  restore_rounding(r);
}
void linear_pcm32_4ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int32_t *dst = (int32_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2le32(s2i(*src[0])); src[0]++;
    dst[1] = int2le32(s2i(*src[1])); src[1]++;
    dst[2] = int2le32(s2i(*src[2])); src[2]++;
    dst[3] = int2le32(s2i(*src[3])); src[3]++;
    dst += 4;
  }
  restore_rounding(r);
}
void linear_pcm16_be_4ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int16_t *dst = (int16_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2be16(s2i(*src[0])); src[0]++;
    dst[1] = int2be16(s2i(*src[1])); src[1]++;
    dst[2] = int2be16(s2i(*src[2])); src[2]++;
    dst[3] = int2be16(s2i(*src[3])); src[3]++;
    dst += 4;
  }
  restore_rounding(r);
}
void linear_pcm24_be_4ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int24_t *dst = (int24_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2be24(s2i(*src[0])); src[0]++;
    dst[1] = int2be24(s2i(*src[1])); src[1]++;
    dst[2] = int2be24(s2i(*src[2])); src[2]++;
    dst[3] = int2be24(s2i(*src[3])); src[3]++;
    dst += 4;
  }
  restore_rounding(r);
}
void linear_pcm32_be_4ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int32_t *dst = (int32_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2be32(s2i(*src[0])); src[0]++;
    dst[1] = int2be32(s2i(*src[1])); src[1]++;
    dst[2] = int2be32(s2i(*src[2])); src[2]++;
    dst[3] = int2be32(s2i(*src[3])); src[3]++;
    dst += 4;
  }
  restore_rounding(r);
}
void linear_pcmfloat_4ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  float *dst = (float *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = float(*src[0]); src[0]++;
    dst[1] = float(*src[1]); src[1]++;
    dst[2] = float(*src[2]); src[2]++;
    dst[3] = float(*src[3]); src[3]++;
    dst += 4;
  }
  restore_rounding(r);
}
void linear_pcmdouble_4ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  double *dst = (double *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = double(*src[0]); src[0]++;
    dst[1] = double(*src[1]); src[1]++;
    dst[2] = double(*src[2]); src[2]++;
    dst[3] = double(*src[3]); src[3]++;
    dst += 4;
  }
  restore_rounding(r);
}
void linear_pcm16_5ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int16_t *dst = (int16_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2le16(s2i(*src[0])); src[0]++;
    dst[1] = int2le16(s2i(*src[1])); src[1]++;
    dst[2] = int2le16(s2i(*src[2])); src[2]++;
    dst[3] = int2le16(s2i(*src[3])); src[3]++;
    dst[4] = int2le16(s2i(*src[4])); src[4]++;
    dst += 5;
  }
  restore_rounding(r);
}
void linear_pcm24_5ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int24_t *dst = (int24_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2le24(s2i(*src[0])); src[0]++;
    dst[1] = int2le24(s2i(*src[1])); src[1]++;
    dst[2] = int2le24(s2i(*src[2])); src[2]++;
    dst[3] = int2le24(s2i(*src[3])); src[3]++;
    dst[4] = int2le24(s2i(*src[4])); src[4]++;
    dst += 5;
  }
  restore_rounding(r);
}
void linear_pcm32_5ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int32_t *dst = (int32_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2le32(s2i(*src[0])); src[0]++;
    dst[1] = int2le32(s2i(*src[1])); src[1]++;
    dst[2] = int2le32(s2i(*src[2])); src[2]++;
    dst[3] = int2le32(s2i(*src[3])); src[3]++;
    dst[4] = int2le32(s2i(*src[4])); src[4]++;
    dst += 5;
  }
  restore_rounding(r);
}
void linear_pcm16_be_5ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int16_t *dst = (int16_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2be16(s2i(*src[0])); src[0]++;
    dst[1] = int2be16(s2i(*src[1])); src[1]++;
    dst[2] = int2be16(s2i(*src[2])); src[2]++;
    dst[3] = int2be16(s2i(*src[3])); src[3]++;
    dst[4] = int2be16(s2i(*src[4])); src[4]++;
    dst += 5;
  }
  restore_rounding(r);
}
void linear_pcm24_be_5ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int24_t *dst = (int24_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2be24(s2i(*src[0])); src[0]++;
    dst[1] = int2be24(s2i(*src[1])); src[1]++;
    dst[2] = int2be24(s2i(*src[2])); src[2]++;
    dst[3] = int2be24(s2i(*src[3])); src[3]++;
    dst[4] = int2be24(s2i(*src[4])); src[4]++;
    dst += 5;
  }
  restore_rounding(r);
}
void linear_pcm32_be_5ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int32_t *dst = (int32_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2be32(s2i(*src[0])); src[0]++;
    dst[1] = int2be32(s2i(*src[1])); src[1]++;
    dst[2] = int2be32(s2i(*src[2])); src[2]++;
    dst[3] = int2be32(s2i(*src[3])); src[3]++;
    dst[4] = int2be32(s2i(*src[4])); src[4]++;
    dst += 5;
  }
  restore_rounding(r);
}
void linear_pcmfloat_5ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  float *dst = (float *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = float(*src[0]); src[0]++;
    dst[1] = float(*src[1]); src[1]++;
    dst[2] = float(*src[2]); src[2]++;
    dst[3] = float(*src[3]); src[3]++;
    dst[4] = float(*src[4]); src[4]++;
    dst += 5;
  }
  restore_rounding(r);
}
void linear_pcmdouble_5ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  double *dst = (double *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = double(*src[0]); src[0]++;
    dst[1] = double(*src[1]); src[1]++;
    dst[2] = double(*src[2]); src[2]++;
    dst[3] = double(*src[3]); src[3]++;
    dst[4] = double(*src[4]); src[4]++;
    dst += 5;
  }
  restore_rounding(r);
}
void linear_pcm16_6ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int16_t *dst = (int16_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2le16(s2i(*src[0])); src[0]++;
    dst[1] = int2le16(s2i(*src[1])); src[1]++;
    dst[2] = int2le16(s2i(*src[2])); src[2]++;
    dst[3] = int2le16(s2i(*src[3])); src[3]++;
    dst[4] = int2le16(s2i(*src[4])); src[4]++;
    dst[5] = int2le16(s2i(*src[5])); src[5]++;
    dst += 6;
  }
  restore_rounding(r);
}
void linear_pcm24_6ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int24_t *dst = (int24_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2le24(s2i(*src[0])); src[0]++;
    dst[1] = int2le24(s2i(*src[1])); src[1]++;
    dst[2] = int2le24(s2i(*src[2])); src[2]++;
    dst[3] = int2le24(s2i(*src[3])); src[3]++;
    dst[4] = int2le24(s2i(*src[4])); src[4]++;
    dst[5] = int2le24(s2i(*src[5])); src[5]++;
    dst += 6;
  }
  restore_rounding(r);
}
void linear_pcm32_6ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int32_t *dst = (int32_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2le32(s2i(*src[0])); src[0]++;
    dst[1] = int2le32(s2i(*src[1])); src[1]++;
    dst[2] = int2le32(s2i(*src[2])); src[2]++;
    dst[3] = int2le32(s2i(*src[3])); src[3]++;
    dst[4] = int2le32(s2i(*src[4])); src[4]++;
    dst[5] = int2le32(s2i(*src[5])); src[5]++;
    dst += 6;
  }
  restore_rounding(r);
}
void linear_pcm16_be_6ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int16_t *dst = (int16_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2be16(s2i(*src[0])); src[0]++;
    dst[1] = int2be16(s2i(*src[1])); src[1]++;
    dst[2] = int2be16(s2i(*src[2])); src[2]++;
    dst[3] = int2be16(s2i(*src[3])); src[3]++;
    dst[4] = int2be16(s2i(*src[4])); src[4]++;
    dst[5] = int2be16(s2i(*src[5])); src[5]++;
    dst += 6;
  }
  restore_rounding(r);
}
void linear_pcm24_be_6ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int24_t *dst = (int24_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2be24(s2i(*src[0])); src[0]++;
    dst[1] = int2be24(s2i(*src[1])); src[1]++;
    dst[2] = int2be24(s2i(*src[2])); src[2]++;
    dst[3] = int2be24(s2i(*src[3])); src[3]++;
    dst[4] = int2be24(s2i(*src[4])); src[4]++;
    dst[5] = int2be24(s2i(*src[5])); src[5]++;
    dst += 6;
  }
  restore_rounding(r);
}
void linear_pcm32_be_6ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  int32_t *dst = (int32_t *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = int2be32(s2i(*src[0])); src[0]++;
    dst[1] = int2be32(s2i(*src[1])); src[1]++;
    dst[2] = int2be32(s2i(*src[2])); src[2]++;
    dst[3] = int2be32(s2i(*src[3])); src[3]++;
    dst[4] = int2be32(s2i(*src[4])); src[4]++;
    dst[5] = int2be32(s2i(*src[5])); src[5]++;
    dst += 6;
  }
  restore_rounding(r);
}
void linear_pcmfloat_6ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  float *dst = (float *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = float(*src[0]); src[0]++;
    dst[1] = float(*src[1]); src[1]++;
    dst[2] = float(*src[2]); src[2]++;
    dst[3] = float(*src[3]); src[3]++;
    dst[4] = float(*src[4]); src[4]++;
    dst[5] = float(*src[5]); src[5]++;
    dst += 6;
  }
  restore_rounding(r);
}
void linear_pcmdouble_6ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src = samples;
  double *dst = (double *)rawdata;

  int r = set_rounding();
  while (size--)
  {
    dst[0] = double(*src[0]); src[0]++;
    dst[1] = double(*src[1]); src[1]++;
    dst[2] = double(*src[2]); src[2]++;
    dst[3] = double(*src[3]); src[3]++;
    dst[4] = double(*src[4]); src[4]++;
    dst[5] = double(*src[5]); src[5]++;
    dst += 6;
  }
  restore_rounding(r);
}
