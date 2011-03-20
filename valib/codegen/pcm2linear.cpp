typedef void (Converter::*convert_t)(uint8_t *rawdata, samples_t samples, size_t size);
static const int pcm2linear_formats[] = { FORMAT_PCM16, FORMAT_PCM24, FORMAT_PCM32, FORMAT_PCM16_BE, FORMAT_PCM24_BE, FORMAT_PCM32_BE, FORMAT_PCMFLOAT, FORMAT_PCMDOUBLE, FORMAT_LPCM20, FORMAT_LPCM24 };

static const convert_t pcm2linear_tbl[NCHANNELS][10] = {
 { pcm16_linear_1ch, pcm24_linear_1ch, pcm32_linear_1ch, pcm16_be_linear_1ch, pcm24_be_linear_1ch, pcm32_be_linear_1ch, pcmfloat_linear_1ch, pcmdouble_linear_1ch, lpcm20_linear_1ch, lpcm24_linear_1ch },
 { pcm16_linear_2ch, pcm24_linear_2ch, pcm32_linear_2ch, pcm16_be_linear_2ch, pcm24_be_linear_2ch, pcm32_be_linear_2ch, pcmfloat_linear_2ch, pcmdouble_linear_2ch, lpcm20_linear_2ch, lpcm24_linear_2ch },
 { pcm16_linear_3ch, pcm24_linear_3ch, pcm32_linear_3ch, pcm16_be_linear_3ch, pcm24_be_linear_3ch, pcm32_be_linear_3ch, pcmfloat_linear_3ch, pcmdouble_linear_3ch, lpcm20_linear_3ch, lpcm24_linear_3ch },
 { pcm16_linear_4ch, pcm24_linear_4ch, pcm32_linear_4ch, pcm16_be_linear_4ch, pcm24_be_linear_4ch, pcm32_be_linear_4ch, pcmfloat_linear_4ch, pcmdouble_linear_4ch, lpcm20_linear_4ch, lpcm24_linear_4ch },
 { pcm16_linear_5ch, pcm24_linear_5ch, pcm32_linear_5ch, pcm16_be_linear_5ch, pcm24_be_linear_5ch, pcm32_be_linear_5ch, pcmfloat_linear_5ch, pcmdouble_linear_5ch, lpcm20_linear_5ch, lpcm24_linear_5ch },
 { pcm16_linear_6ch, pcm24_linear_6ch, pcm32_linear_6ch, pcm16_be_linear_6ch, pcm24_be_linear_6ch, pcm32_be_linear_6ch, pcmfloat_linear_6ch, pcmdouble_linear_6ch, lpcm20_linear_6ch, lpcm24_linear_6ch },
};

void pcm16_linear_1ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int16_t *src = (int16_t *)rawdata;
    *dst[0] = i2s(le2int16(src[0])); dst[0]++;
    rawdata += 2;
  }
}
void pcm24_linear_1ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int24_t *src = (int24_t *)rawdata;
    *dst[0] = i2s(le2int24(src[0])); dst[0]++;
    rawdata += 3;
  }
}
void pcm32_linear_1ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int32_t *src = (int32_t *)rawdata;
    *dst[0] = i2s(le2int32(src[0])); dst[0]++;
    rawdata += 4;
  }
}
void pcm16_be_linear_1ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int16_t *src = (int16_t *)rawdata;
    *dst[0] = i2s(be2int16(src[0])); dst[0]++;
    rawdata += 2;
  }
}
void pcm24_be_linear_1ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int24_t *src = (int24_t *)rawdata;
    *dst[0] = i2s(be2int24(src[0])); dst[0]++;
    rawdata += 3;
  }
}
void pcm32_be_linear_1ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int32_t *src = (int32_t *)rawdata;
    *dst[0] = i2s(be2int32(src[0])); dst[0]++;
    rawdata += 4;
  }
}
void pcmfloat_linear_1ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    float *src = (float *)rawdata;
    *dst[0] = sample_t(src[0]); dst[0]++;
    rawdata += 4;
  }
}
void pcmdouble_linear_1ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    double *src = (double *)rawdata;
    *dst[0] = sample_t(src[0]); dst[0]++;
    rawdata += 8;
  }
}
void lpcm20_linear_1ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int16_t *src = (int16_t *)rawdata;
    dst[0][0] = i2s(int32_t(be2int16(src[0+1*0])) << 4); dst[0][1] = i2s(int32_t(be2int16(src[0+1*1])) << 4); dst[0]+=2;
    rawdata += 5;
  }
}
void lpcm24_linear_1ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int16_t *src = (int16_t *)rawdata;
    dst[0][0] = i2s(int32_t(be2int16(src[0+1*0]) << 8) | rawdata[1*4+0+1*0]); dst[0][1] = i2s(int32_t(be2int16(src[0+1*1]) << 8) | rawdata[1*4+0+1*1]); dst[0]+=2;
    rawdata += 6;
  }
}
void pcm16_linear_2ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int16_t *src = (int16_t *)rawdata;
    *dst[0] = i2s(le2int16(src[0])); dst[0]++;
    *dst[1] = i2s(le2int16(src[1])); dst[1]++;
    rawdata += 4;
  }
}
void pcm24_linear_2ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int24_t *src = (int24_t *)rawdata;
    *dst[0] = i2s(le2int24(src[0])); dst[0]++;
    *dst[1] = i2s(le2int24(src[1])); dst[1]++;
    rawdata += 6;
  }
}
void pcm32_linear_2ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int32_t *src = (int32_t *)rawdata;
    *dst[0] = i2s(le2int32(src[0])); dst[0]++;
    *dst[1] = i2s(le2int32(src[1])); dst[1]++;
    rawdata += 8;
  }
}
void pcm16_be_linear_2ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int16_t *src = (int16_t *)rawdata;
    *dst[0] = i2s(be2int16(src[0])); dst[0]++;
    *dst[1] = i2s(be2int16(src[1])); dst[1]++;
    rawdata += 4;
  }
}
void pcm24_be_linear_2ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int24_t *src = (int24_t *)rawdata;
    *dst[0] = i2s(be2int24(src[0])); dst[0]++;
    *dst[1] = i2s(be2int24(src[1])); dst[1]++;
    rawdata += 6;
  }
}
void pcm32_be_linear_2ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int32_t *src = (int32_t *)rawdata;
    *dst[0] = i2s(be2int32(src[0])); dst[0]++;
    *dst[1] = i2s(be2int32(src[1])); dst[1]++;
    rawdata += 8;
  }
}
void pcmfloat_linear_2ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    float *src = (float *)rawdata;
    *dst[0] = sample_t(src[0]); dst[0]++;
    *dst[1] = sample_t(src[1]); dst[1]++;
    rawdata += 8;
  }
}
void pcmdouble_linear_2ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    double *src = (double *)rawdata;
    *dst[0] = sample_t(src[0]); dst[0]++;
    *dst[1] = sample_t(src[1]); dst[1]++;
    rawdata += 16;
  }
}
void lpcm20_linear_2ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int16_t *src = (int16_t *)rawdata;
    dst[0][0] = i2s(int32_t(be2int16(src[0+2*0])) << 4); dst[0][1] = i2s(int32_t(be2int16(src[0+2*1])) << 4); dst[0]+=2;
    dst[1][0] = i2s(int32_t(be2int16(src[1+2*0])) << 4); dst[1][1] = i2s(int32_t(be2int16(src[1+2*1])) << 4); dst[1]+=2;
    rawdata += 10;
  }
}
void lpcm24_linear_2ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int16_t *src = (int16_t *)rawdata;
    dst[0][0] = i2s(int32_t(be2int16(src[0+2*0]) << 8) | rawdata[2*4+0+2*0]); dst[0][1] = i2s(int32_t(be2int16(src[0+2*1]) << 8) | rawdata[2*4+0+2*1]); dst[0]+=2;
    dst[1][0] = i2s(int32_t(be2int16(src[1+2*0]) << 8) | rawdata[2*4+1+2*0]); dst[1][1] = i2s(int32_t(be2int16(src[1+2*1]) << 8) | rawdata[2*4+1+2*1]); dst[1]+=2;
    rawdata += 12;
  }
}
void pcm16_linear_3ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int16_t *src = (int16_t *)rawdata;
    *dst[0] = i2s(le2int16(src[0])); dst[0]++;
    *dst[1] = i2s(le2int16(src[1])); dst[1]++;
    *dst[2] = i2s(le2int16(src[2])); dst[2]++;
    rawdata += 6;
  }
}
void pcm24_linear_3ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int24_t *src = (int24_t *)rawdata;
    *dst[0] = i2s(le2int24(src[0])); dst[0]++;
    *dst[1] = i2s(le2int24(src[1])); dst[1]++;
    *dst[2] = i2s(le2int24(src[2])); dst[2]++;
    rawdata += 9;
  }
}
void pcm32_linear_3ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int32_t *src = (int32_t *)rawdata;
    *dst[0] = i2s(le2int32(src[0])); dst[0]++;
    *dst[1] = i2s(le2int32(src[1])); dst[1]++;
    *dst[2] = i2s(le2int32(src[2])); dst[2]++;
    rawdata += 12;
  }
}
void pcm16_be_linear_3ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int16_t *src = (int16_t *)rawdata;
    *dst[0] = i2s(be2int16(src[0])); dst[0]++;
    *dst[1] = i2s(be2int16(src[1])); dst[1]++;
    *dst[2] = i2s(be2int16(src[2])); dst[2]++;
    rawdata += 6;
  }
}
void pcm24_be_linear_3ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int24_t *src = (int24_t *)rawdata;
    *dst[0] = i2s(be2int24(src[0])); dst[0]++;
    *dst[1] = i2s(be2int24(src[1])); dst[1]++;
    *dst[2] = i2s(be2int24(src[2])); dst[2]++;
    rawdata += 9;
  }
}
void pcm32_be_linear_3ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int32_t *src = (int32_t *)rawdata;
    *dst[0] = i2s(be2int32(src[0])); dst[0]++;
    *dst[1] = i2s(be2int32(src[1])); dst[1]++;
    *dst[2] = i2s(be2int32(src[2])); dst[2]++;
    rawdata += 12;
  }
}
void pcmfloat_linear_3ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    float *src = (float *)rawdata;
    *dst[0] = sample_t(src[0]); dst[0]++;
    *dst[1] = sample_t(src[1]); dst[1]++;
    *dst[2] = sample_t(src[2]); dst[2]++;
    rawdata += 12;
  }
}
void pcmdouble_linear_3ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    double *src = (double *)rawdata;
    *dst[0] = sample_t(src[0]); dst[0]++;
    *dst[1] = sample_t(src[1]); dst[1]++;
    *dst[2] = sample_t(src[2]); dst[2]++;
    rawdata += 24;
  }
}
void lpcm20_linear_3ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int16_t *src = (int16_t *)rawdata;
    dst[0][0] = i2s(int32_t(be2int16(src[0+3*0])) << 4); dst[0][1] = i2s(int32_t(be2int16(src[0+3*1])) << 4); dst[0]+=2;
    dst[1][0] = i2s(int32_t(be2int16(src[1+3*0])) << 4); dst[1][1] = i2s(int32_t(be2int16(src[1+3*1])) << 4); dst[1]+=2;
    dst[2][0] = i2s(int32_t(be2int16(src[2+3*0])) << 4); dst[2][1] = i2s(int32_t(be2int16(src[2+3*1])) << 4); dst[2]+=2;
    rawdata += 15;
  }
}
void lpcm24_linear_3ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int16_t *src = (int16_t *)rawdata;
    dst[0][0] = i2s(int32_t(be2int16(src[0+3*0]) << 8) | rawdata[3*4+0+3*0]); dst[0][1] = i2s(int32_t(be2int16(src[0+3*1]) << 8) | rawdata[3*4+0+3*1]); dst[0]+=2;
    dst[1][0] = i2s(int32_t(be2int16(src[1+3*0]) << 8) | rawdata[3*4+1+3*0]); dst[1][1] = i2s(int32_t(be2int16(src[1+3*1]) << 8) | rawdata[3*4+1+3*1]); dst[1]+=2;
    dst[2][0] = i2s(int32_t(be2int16(src[2+3*0]) << 8) | rawdata[3*4+2+3*0]); dst[2][1] = i2s(int32_t(be2int16(src[2+3*1]) << 8) | rawdata[3*4+2+3*1]); dst[2]+=2;
    rawdata += 18;
  }
}
void pcm16_linear_4ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int16_t *src = (int16_t *)rawdata;
    *dst[0] = i2s(le2int16(src[0])); dst[0]++;
    *dst[1] = i2s(le2int16(src[1])); dst[1]++;
    *dst[2] = i2s(le2int16(src[2])); dst[2]++;
    *dst[3] = i2s(le2int16(src[3])); dst[3]++;
    rawdata += 8;
  }
}
void pcm24_linear_4ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int24_t *src = (int24_t *)rawdata;
    *dst[0] = i2s(le2int24(src[0])); dst[0]++;
    *dst[1] = i2s(le2int24(src[1])); dst[1]++;
    *dst[2] = i2s(le2int24(src[2])); dst[2]++;
    *dst[3] = i2s(le2int24(src[3])); dst[3]++;
    rawdata += 12;
  }
}
void pcm32_linear_4ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int32_t *src = (int32_t *)rawdata;
    *dst[0] = i2s(le2int32(src[0])); dst[0]++;
    *dst[1] = i2s(le2int32(src[1])); dst[1]++;
    *dst[2] = i2s(le2int32(src[2])); dst[2]++;
    *dst[3] = i2s(le2int32(src[3])); dst[3]++;
    rawdata += 16;
  }
}
void pcm16_be_linear_4ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int16_t *src = (int16_t *)rawdata;
    *dst[0] = i2s(be2int16(src[0])); dst[0]++;
    *dst[1] = i2s(be2int16(src[1])); dst[1]++;
    *dst[2] = i2s(be2int16(src[2])); dst[2]++;
    *dst[3] = i2s(be2int16(src[3])); dst[3]++;
    rawdata += 8;
  }
}
void pcm24_be_linear_4ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int24_t *src = (int24_t *)rawdata;
    *dst[0] = i2s(be2int24(src[0])); dst[0]++;
    *dst[1] = i2s(be2int24(src[1])); dst[1]++;
    *dst[2] = i2s(be2int24(src[2])); dst[2]++;
    *dst[3] = i2s(be2int24(src[3])); dst[3]++;
    rawdata += 12;
  }
}
void pcm32_be_linear_4ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int32_t *src = (int32_t *)rawdata;
    *dst[0] = i2s(be2int32(src[0])); dst[0]++;
    *dst[1] = i2s(be2int32(src[1])); dst[1]++;
    *dst[2] = i2s(be2int32(src[2])); dst[2]++;
    *dst[3] = i2s(be2int32(src[3])); dst[3]++;
    rawdata += 16;
  }
}
void pcmfloat_linear_4ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    float *src = (float *)rawdata;
    *dst[0] = sample_t(src[0]); dst[0]++;
    *dst[1] = sample_t(src[1]); dst[1]++;
    *dst[2] = sample_t(src[2]); dst[2]++;
    *dst[3] = sample_t(src[3]); dst[3]++;
    rawdata += 16;
  }
}
void pcmdouble_linear_4ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    double *src = (double *)rawdata;
    *dst[0] = sample_t(src[0]); dst[0]++;
    *dst[1] = sample_t(src[1]); dst[1]++;
    *dst[2] = sample_t(src[2]); dst[2]++;
    *dst[3] = sample_t(src[3]); dst[3]++;
    rawdata += 32;
  }
}
void lpcm20_linear_4ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int16_t *src = (int16_t *)rawdata;
    dst[0][0] = i2s(int32_t(be2int16(src[0+4*0])) << 4); dst[0][1] = i2s(int32_t(be2int16(src[0+4*1])) << 4); dst[0]+=2;
    dst[1][0] = i2s(int32_t(be2int16(src[1+4*0])) << 4); dst[1][1] = i2s(int32_t(be2int16(src[1+4*1])) << 4); dst[1]+=2;
    dst[2][0] = i2s(int32_t(be2int16(src[2+4*0])) << 4); dst[2][1] = i2s(int32_t(be2int16(src[2+4*1])) << 4); dst[2]+=2;
    dst[3][0] = i2s(int32_t(be2int16(src[3+4*0])) << 4); dst[3][1] = i2s(int32_t(be2int16(src[3+4*1])) << 4); dst[3]+=2;
    rawdata += 20;
  }
}
void lpcm24_linear_4ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int16_t *src = (int16_t *)rawdata;
    dst[0][0] = i2s(int32_t(be2int16(src[0+4*0]) << 8) | rawdata[4*4+0+4*0]); dst[0][1] = i2s(int32_t(be2int16(src[0+4*1]) << 8) | rawdata[4*4+0+4*1]); dst[0]+=2;
    dst[1][0] = i2s(int32_t(be2int16(src[1+4*0]) << 8) | rawdata[4*4+1+4*0]); dst[1][1] = i2s(int32_t(be2int16(src[1+4*1]) << 8) | rawdata[4*4+1+4*1]); dst[1]+=2;
    dst[2][0] = i2s(int32_t(be2int16(src[2+4*0]) << 8) | rawdata[4*4+2+4*0]); dst[2][1] = i2s(int32_t(be2int16(src[2+4*1]) << 8) | rawdata[4*4+2+4*1]); dst[2]+=2;
    dst[3][0] = i2s(int32_t(be2int16(src[3+4*0]) << 8) | rawdata[4*4+3+4*0]); dst[3][1] = i2s(int32_t(be2int16(src[3+4*1]) << 8) | rawdata[4*4+3+4*1]); dst[3]+=2;
    rawdata += 24;
  }
}
void pcm16_linear_5ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int16_t *src = (int16_t *)rawdata;
    *dst[0] = i2s(le2int16(src[0])); dst[0]++;
    *dst[1] = i2s(le2int16(src[1])); dst[1]++;
    *dst[2] = i2s(le2int16(src[2])); dst[2]++;
    *dst[3] = i2s(le2int16(src[3])); dst[3]++;
    *dst[4] = i2s(le2int16(src[4])); dst[4]++;
    rawdata += 10;
  }
}
void pcm24_linear_5ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int24_t *src = (int24_t *)rawdata;
    *dst[0] = i2s(le2int24(src[0])); dst[0]++;
    *dst[1] = i2s(le2int24(src[1])); dst[1]++;
    *dst[2] = i2s(le2int24(src[2])); dst[2]++;
    *dst[3] = i2s(le2int24(src[3])); dst[3]++;
    *dst[4] = i2s(le2int24(src[4])); dst[4]++;
    rawdata += 15;
  }
}
void pcm32_linear_5ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int32_t *src = (int32_t *)rawdata;
    *dst[0] = i2s(le2int32(src[0])); dst[0]++;
    *dst[1] = i2s(le2int32(src[1])); dst[1]++;
    *dst[2] = i2s(le2int32(src[2])); dst[2]++;
    *dst[3] = i2s(le2int32(src[3])); dst[3]++;
    *dst[4] = i2s(le2int32(src[4])); dst[4]++;
    rawdata += 20;
  }
}
void pcm16_be_linear_5ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int16_t *src = (int16_t *)rawdata;
    *dst[0] = i2s(be2int16(src[0])); dst[0]++;
    *dst[1] = i2s(be2int16(src[1])); dst[1]++;
    *dst[2] = i2s(be2int16(src[2])); dst[2]++;
    *dst[3] = i2s(be2int16(src[3])); dst[3]++;
    *dst[4] = i2s(be2int16(src[4])); dst[4]++;
    rawdata += 10;
  }
}
void pcm24_be_linear_5ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int24_t *src = (int24_t *)rawdata;
    *dst[0] = i2s(be2int24(src[0])); dst[0]++;
    *dst[1] = i2s(be2int24(src[1])); dst[1]++;
    *dst[2] = i2s(be2int24(src[2])); dst[2]++;
    *dst[3] = i2s(be2int24(src[3])); dst[3]++;
    *dst[4] = i2s(be2int24(src[4])); dst[4]++;
    rawdata += 15;
  }
}
void pcm32_be_linear_5ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int32_t *src = (int32_t *)rawdata;
    *dst[0] = i2s(be2int32(src[0])); dst[0]++;
    *dst[1] = i2s(be2int32(src[1])); dst[1]++;
    *dst[2] = i2s(be2int32(src[2])); dst[2]++;
    *dst[3] = i2s(be2int32(src[3])); dst[3]++;
    *dst[4] = i2s(be2int32(src[4])); dst[4]++;
    rawdata += 20;
  }
}
void pcmfloat_linear_5ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    float *src = (float *)rawdata;
    *dst[0] = sample_t(src[0]); dst[0]++;
    *dst[1] = sample_t(src[1]); dst[1]++;
    *dst[2] = sample_t(src[2]); dst[2]++;
    *dst[3] = sample_t(src[3]); dst[3]++;
    *dst[4] = sample_t(src[4]); dst[4]++;
    rawdata += 20;
  }
}
void pcmdouble_linear_5ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    double *src = (double *)rawdata;
    *dst[0] = sample_t(src[0]); dst[0]++;
    *dst[1] = sample_t(src[1]); dst[1]++;
    *dst[2] = sample_t(src[2]); dst[2]++;
    *dst[3] = sample_t(src[3]); dst[3]++;
    *dst[4] = sample_t(src[4]); dst[4]++;
    rawdata += 40;
  }
}
void lpcm20_linear_5ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int16_t *src = (int16_t *)rawdata;
    dst[0][0] = i2s(int32_t(be2int16(src[0+5*0])) << 4); dst[0][1] = i2s(int32_t(be2int16(src[0+5*1])) << 4); dst[0]+=2;
    dst[1][0] = i2s(int32_t(be2int16(src[1+5*0])) << 4); dst[1][1] = i2s(int32_t(be2int16(src[1+5*1])) << 4); dst[1]+=2;
    dst[2][0] = i2s(int32_t(be2int16(src[2+5*0])) << 4); dst[2][1] = i2s(int32_t(be2int16(src[2+5*1])) << 4); dst[2]+=2;
    dst[3][0] = i2s(int32_t(be2int16(src[3+5*0])) << 4); dst[3][1] = i2s(int32_t(be2int16(src[3+5*1])) << 4); dst[3]+=2;
    dst[4][0] = i2s(int32_t(be2int16(src[4+5*0])) << 4); dst[4][1] = i2s(int32_t(be2int16(src[4+5*1])) << 4); dst[4]+=2;
    rawdata += 25;
  }
}
void lpcm24_linear_5ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int16_t *src = (int16_t *)rawdata;
    dst[0][0] = i2s(int32_t(be2int16(src[0+5*0]) << 8) | rawdata[5*4+0+5*0]); dst[0][1] = i2s(int32_t(be2int16(src[0+5*1]) << 8) | rawdata[5*4+0+5*1]); dst[0]+=2;
    dst[1][0] = i2s(int32_t(be2int16(src[1+5*0]) << 8) | rawdata[5*4+1+5*0]); dst[1][1] = i2s(int32_t(be2int16(src[1+5*1]) << 8) | rawdata[5*4+1+5*1]); dst[1]+=2;
    dst[2][0] = i2s(int32_t(be2int16(src[2+5*0]) << 8) | rawdata[5*4+2+5*0]); dst[2][1] = i2s(int32_t(be2int16(src[2+5*1]) << 8) | rawdata[5*4+2+5*1]); dst[2]+=2;
    dst[3][0] = i2s(int32_t(be2int16(src[3+5*0]) << 8) | rawdata[5*4+3+5*0]); dst[3][1] = i2s(int32_t(be2int16(src[3+5*1]) << 8) | rawdata[5*4+3+5*1]); dst[3]+=2;
    dst[4][0] = i2s(int32_t(be2int16(src[4+5*0]) << 8) | rawdata[5*4+4+5*0]); dst[4][1] = i2s(int32_t(be2int16(src[4+5*1]) << 8) | rawdata[5*4+4+5*1]); dst[4]+=2;
    rawdata += 30;
  }
}
void pcm16_linear_6ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int16_t *src = (int16_t *)rawdata;
    *dst[0] = i2s(le2int16(src[0])); dst[0]++;
    *dst[1] = i2s(le2int16(src[1])); dst[1]++;
    *dst[2] = i2s(le2int16(src[2])); dst[2]++;
    *dst[3] = i2s(le2int16(src[3])); dst[3]++;
    *dst[4] = i2s(le2int16(src[4])); dst[4]++;
    *dst[5] = i2s(le2int16(src[5])); dst[5]++;
    rawdata += 12;
  }
}
void pcm24_linear_6ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int24_t *src = (int24_t *)rawdata;
    *dst[0] = i2s(le2int24(src[0])); dst[0]++;
    *dst[1] = i2s(le2int24(src[1])); dst[1]++;
    *dst[2] = i2s(le2int24(src[2])); dst[2]++;
    *dst[3] = i2s(le2int24(src[3])); dst[3]++;
    *dst[4] = i2s(le2int24(src[4])); dst[4]++;
    *dst[5] = i2s(le2int24(src[5])); dst[5]++;
    rawdata += 18;
  }
}
void pcm32_linear_6ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int32_t *src = (int32_t *)rawdata;
    *dst[0] = i2s(le2int32(src[0])); dst[0]++;
    *dst[1] = i2s(le2int32(src[1])); dst[1]++;
    *dst[2] = i2s(le2int32(src[2])); dst[2]++;
    *dst[3] = i2s(le2int32(src[3])); dst[3]++;
    *dst[4] = i2s(le2int32(src[4])); dst[4]++;
    *dst[5] = i2s(le2int32(src[5])); dst[5]++;
    rawdata += 24;
  }
}
void pcm16_be_linear_6ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int16_t *src = (int16_t *)rawdata;
    *dst[0] = i2s(be2int16(src[0])); dst[0]++;
    *dst[1] = i2s(be2int16(src[1])); dst[1]++;
    *dst[2] = i2s(be2int16(src[2])); dst[2]++;
    *dst[3] = i2s(be2int16(src[3])); dst[3]++;
    *dst[4] = i2s(be2int16(src[4])); dst[4]++;
    *dst[5] = i2s(be2int16(src[5])); dst[5]++;
    rawdata += 12;
  }
}
void pcm24_be_linear_6ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int24_t *src = (int24_t *)rawdata;
    *dst[0] = i2s(be2int24(src[0])); dst[0]++;
    *dst[1] = i2s(be2int24(src[1])); dst[1]++;
    *dst[2] = i2s(be2int24(src[2])); dst[2]++;
    *dst[3] = i2s(be2int24(src[3])); dst[3]++;
    *dst[4] = i2s(be2int24(src[4])); dst[4]++;
    *dst[5] = i2s(be2int24(src[5])); dst[5]++;
    rawdata += 18;
  }
}
void pcm32_be_linear_6ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int32_t *src = (int32_t *)rawdata;
    *dst[0] = i2s(be2int32(src[0])); dst[0]++;
    *dst[1] = i2s(be2int32(src[1])); dst[1]++;
    *dst[2] = i2s(be2int32(src[2])); dst[2]++;
    *dst[3] = i2s(be2int32(src[3])); dst[3]++;
    *dst[4] = i2s(be2int32(src[4])); dst[4]++;
    *dst[5] = i2s(be2int32(src[5])); dst[5]++;
    rawdata += 24;
  }
}
void pcmfloat_linear_6ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    float *src = (float *)rawdata;
    *dst[0] = sample_t(src[0]); dst[0]++;
    *dst[1] = sample_t(src[1]); dst[1]++;
    *dst[2] = sample_t(src[2]); dst[2]++;
    *dst[3] = sample_t(src[3]); dst[3]++;
    *dst[4] = sample_t(src[4]); dst[4]++;
    *dst[5] = sample_t(src[5]); dst[5]++;
    rawdata += 24;
  }
}
void pcmdouble_linear_6ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    double *src = (double *)rawdata;
    *dst[0] = sample_t(src[0]); dst[0]++;
    *dst[1] = sample_t(src[1]); dst[1]++;
    *dst[2] = sample_t(src[2]); dst[2]++;
    *dst[3] = sample_t(src[3]); dst[3]++;
    *dst[4] = sample_t(src[4]); dst[4]++;
    *dst[5] = sample_t(src[5]); dst[5]++;
    rawdata += 48;
  }
}
void lpcm20_linear_6ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int16_t *src = (int16_t *)rawdata;
    dst[0][0] = i2s(int32_t(be2int16(src[0+6*0])) << 4); dst[0][1] = i2s(int32_t(be2int16(src[0+6*1])) << 4); dst[0]+=2;
    dst[1][0] = i2s(int32_t(be2int16(src[1+6*0])) << 4); dst[1][1] = i2s(int32_t(be2int16(src[1+6*1])) << 4); dst[1]+=2;
    dst[2][0] = i2s(int32_t(be2int16(src[2+6*0])) << 4); dst[2][1] = i2s(int32_t(be2int16(src[2+6*1])) << 4); dst[2]+=2;
    dst[3][0] = i2s(int32_t(be2int16(src[3+6*0])) << 4); dst[3][1] = i2s(int32_t(be2int16(src[3+6*1])) << 4); dst[3]+=2;
    dst[4][0] = i2s(int32_t(be2int16(src[4+6*0])) << 4); dst[4][1] = i2s(int32_t(be2int16(src[4+6*1])) << 4); dst[4]+=2;
    dst[5][0] = i2s(int32_t(be2int16(src[5+6*0])) << 4); dst[5][1] = i2s(int32_t(be2int16(src[5+6*1])) << 4); dst[5]+=2;
    rawdata += 30;
  }
}
void lpcm24_linear_6ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t dst = samples;
  while (size--)
  {
    int16_t *src = (int16_t *)rawdata;
    dst[0][0] = i2s(int32_t(be2int16(src[0+6*0]) << 8) | rawdata[6*4+0+6*0]); dst[0][1] = i2s(int32_t(be2int16(src[0+6*1]) << 8) | rawdata[6*4+0+6*1]); dst[0]+=2;
    dst[1][0] = i2s(int32_t(be2int16(src[1+6*0]) << 8) | rawdata[6*4+1+6*0]); dst[1][1] = i2s(int32_t(be2int16(src[1+6*1]) << 8) | rawdata[6*4+1+6*1]); dst[1]+=2;
    dst[2][0] = i2s(int32_t(be2int16(src[2+6*0]) << 8) | rawdata[6*4+2+6*0]); dst[2][1] = i2s(int32_t(be2int16(src[2+6*1]) << 8) | rawdata[6*4+2+6*1]); dst[2]+=2;
    dst[3][0] = i2s(int32_t(be2int16(src[3+6*0]) << 8) | rawdata[6*4+3+6*0]); dst[3][1] = i2s(int32_t(be2int16(src[3+6*1]) << 8) | rawdata[6*4+3+6*1]); dst[3]+=2;
    dst[4][0] = i2s(int32_t(be2int16(src[4+6*0]) << 8) | rawdata[6*4+4+6*0]); dst[4][1] = i2s(int32_t(be2int16(src[4+6*1]) << 8) | rawdata[6*4+4+6*1]); dst[4]+=2;
    dst[5][0] = i2s(int32_t(be2int16(src[5+6*0]) << 8) | rawdata[6*4+5+6*0]); dst[5][1] = i2s(int32_t(be2int16(src[5+6*1]) << 8) | rawdata[6*4+5+6*1]); dst[5]+=2;
    rawdata += 36;
  }
}
