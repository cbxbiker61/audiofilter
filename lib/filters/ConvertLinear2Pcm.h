#pragma once
#ifndef AUDIOFILTER_CONVERTLINEAR2PCM_H
#define AUDIOFILTER_CONVERTLINEAR2PCM_H

/* included from ConvertFunc.cpp */

namespace AudioFilter {

void linear_pcm16_1ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src(samples);
  int16_t *dst((int16_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
  {
    dst[0] = int2le16(s2i(*src[0])); src[0]++;
    dst += 1;
  }

  restore_rounding(r);
}

void linear_pcm24_1ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src(samples);
  int24_t *dst((int24_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
  {
    dst[0] = int2le24(s2i(*src[0])); src[0]++;
    dst += 1;
  }

  restore_rounding(r);
}

void linear_pcm32_1ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src(samples);
  int32_t *dst((int32_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
  {
    dst[0] = int2le32(s2i(*src[0])); src[0]++;
    dst += 1;
  }

  restore_rounding(r);
}

void linear_pcm16_be_1ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src(samples);
  int16_t *dst((int16_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
  {
    dst[0] = int2be16(s2i(*src[0])); src[0]++;
    dst += 1;
  }

  restore_rounding(r);
}

void linear_pcm24_be_1ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src(samples);
  int24_t *dst((int24_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
  {
    dst[0] = int2be24(s2i(*src[0])); src[0]++;
    dst += 1;
  }

  restore_rounding(r);
}

void linear_pcm32_be_1ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src(samples);
  int32_t *dst((int32_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
  {
    dst[0] = int2be32(s2i(*src[0])); src[0]++;
    dst += 1;
  }

  restore_rounding(r);
}

void linear_pcmfloat_1ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src(samples);
  float *dst((float *)rawdata);
  const int r(set_rounding());

  while ( size-- )
  {
    dst[0] = float(*src[0]); src[0]++;
    dst += 1;
  }

  restore_rounding(r);
}

void linear_pcmdouble_1ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src(samples);
  double *dst((double *)rawdata);
  const int r(set_rounding());

  while ( size-- )
  {
    dst[0] = double(*src[0]); src[0]++;
    dst += 1;
  }

  restore_rounding(r);
}

void linear_pcm16_2ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src(samples);
  int16_t *dst((int16_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
  {
    dst[0] = int2le16(s2i(*src[0])); src[0]++;
    dst[1] = int2le16(s2i(*src[1])); src[1]++;
    dst += 2;
  }

  restore_rounding(r);
}

void linear_pcm24_2ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src(samples);
  int24_t *dst((int24_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
  {
    dst[0] = int2le24(s2i(*src[0])); src[0]++;
    dst[1] = int2le24(s2i(*src[1])); src[1]++;
    dst += 2;
  }

  restore_rounding(r);
}

void linear_pcm32_2ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src(samples);
  int32_t *dst((int32_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
  {
    dst[0] = int2le32(s2i(*src[0])); src[0]++;
    dst[1] = int2le32(s2i(*src[1])); src[1]++;
    dst += 2;
  }

  restore_rounding(r);
}

void linear_pcm16_be_2ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src(samples);
  int16_t *dst((int16_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
  {
    dst[0] = int2be16(s2i(*src[0])); src[0]++;
    dst[1] = int2be16(s2i(*src[1])); src[1]++;
    dst += 2;
  }

  restore_rounding(r);
}

void linear_pcm24_be_2ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src(samples);
  int24_t *dst((int24_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
  {
    dst[0] = int2be24(s2i(*src[0])); src[0]++;
    dst[1] = int2be24(s2i(*src[1])); src[1]++;
    dst += 2;
  }

  restore_rounding(r);
}

void linear_pcm32_be_2ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src(samples);
  int32_t *dst((int32_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
  {
    dst[0] = int2be32(s2i(*src[0])); src[0]++;
    dst[1] = int2be32(s2i(*src[1])); src[1]++;
    dst += 2;
  }

  restore_rounding(r);
}

void linear_pcmfloat_2ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src(samples);
  float *dst((float *)rawdata);
  const int r(set_rounding());

  while ( size-- )
  {
    dst[0] = float(*src[0]); src[0]++;
    dst[1] = float(*src[1]); src[1]++;
    dst += 2;
  }

  restore_rounding(r);
}

void linear_pcmdouble_2ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src(samples);
  double *dst((double *)rawdata);
  const int r(set_rounding());

  while ( size-- )
  {
    dst[0] = double(*src[0]); src[0]++;
    dst[1] = double(*src[1]); src[1]++;
    dst += 2;
  }

  restore_rounding(r);
}

void linear_pcm16_3ch(uint8_t *rawdata, samples_t samples, size_t size)
{
  samples_t src(samples);
  int16_t *dst((int16_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
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
  samples_t src(samples);
  int24_t *dst((int24_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
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
  samples_t src(samples);
  int32_t *dst((int32_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
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
  samples_t src(samples);
  int16_t *dst((int16_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
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
  samples_t src(samples);
  int24_t *dst((int24_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
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
  samples_t src(samples);
  int32_t *dst((int32_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
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
  samples_t src(samples);
  float *dst((float *)rawdata);
  const int r(set_rounding());

  while ( size-- )
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
  samples_t src(samples);
  double *dst((double *)rawdata);
  const int r(set_rounding());

  while ( size-- )
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
  samples_t src(samples);
  int16_t *dst((int16_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
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
  samples_t src(samples);
  int24_t *dst((int24_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
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
  samples_t src(samples);
  int32_t *dst((int32_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
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
  samples_t src(samples);
  int16_t *dst((int16_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
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
  samples_t src(samples);
  int24_t *dst((int24_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
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
  samples_t src(samples);
  int32_t *dst((int32_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
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
  samples_t src(samples);
  float *dst((float *)rawdata);
  const int r(set_rounding());

  while ( size-- )
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
  samples_t src(samples);
  double *dst((double *)rawdata);
  const int r(set_rounding());

  while ( size-- )
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
  samples_t src(samples);
  int16_t *dst((int16_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
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
  samples_t src(samples);
  int24_t *dst((int24_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
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
  samples_t src(samples);
  int32_t *dst((int32_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
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
  samples_t src(samples);
  int16_t *dst((int16_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
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
  samples_t src(samples);
  int24_t *dst((int24_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
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
  samples_t src(samples);
  int32_t *dst((int32_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
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
  samples_t src(samples);
  float *dst((float *)rawdata);
  const int r(set_rounding());

  while ( size-- )
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
  samples_t src(samples);
  double *dst((double *)rawdata);
  const int r(set_rounding());

  while ( size-- )
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
  samples_t src(samples);
  int16_t *dst((int16_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
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
  samples_t src(samples);
  int24_t *dst((int24_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
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
  samples_t src(samples);
  int32_t *dst((int32_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
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
  samples_t src(samples);
  int16_t *dst((int16_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
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
  samples_t src(samples);
  int24_t *dst((int24_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
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
  samples_t src(samples);
  int32_t *dst((int32_t *)rawdata);
  const int r(set_rounding());

  while ( size-- )
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
  samples_t src(samples);
  float *dst((float *)rawdata);
  const int r(set_rounding());

  while ( size-- )
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
  samples_t src(samples);
  double *dst((double *)rawdata);
  const int r(set_rounding());

  while ( size-- )
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

}; // namespace AudioFilter

#endif

// vim: ts=2 sts=2 et

