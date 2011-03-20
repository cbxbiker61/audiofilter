#include "ac3_imdct.h"

const sample_t imdct_window[] =
{
  0.00014, 0.00024, 0.00037, 0.00051, 0.00067, 0.00086, 0.00107, 0.00130,
  0.00157, 0.00187, 0.00220, 0.00256, 0.00297, 0.00341, 0.00390, 0.00443,
  0.00501, 0.00564, 0.00632, 0.00706, 0.00785, 0.00871, 0.00962, 0.01061,
  0.01166, 0.01279, 0.01399, 0.01526, 0.01662, 0.01806, 0.01959, 0.02121,
  0.02292, 0.02472, 0.02662, 0.02863, 0.03073, 0.03294, 0.03527, 0.03770,
  0.04025, 0.04292, 0.04571, 0.04862, 0.05165, 0.05481, 0.05810, 0.06153,
  0.06508, 0.06878, 0.07261, 0.07658, 0.08069, 0.08495, 0.08935, 0.09389,
  0.09859, 0.10343, 0.10842, 0.11356, 0.11885, 0.12429, 0.12988, 0.13563,
  0.14152, 0.14757, 0.15376, 0.16011, 0.16661, 0.17325, 0.18005, 0.18699,
  0.19407, 0.20130, 0.20867, 0.21618, 0.22382, 0.23161, 0.23952, 0.24757,
  0.25574, 0.26404, 0.27246, 0.28100, 0.28965, 0.29841, 0.30729, 0.31626,
  0.32533, 0.33450, 0.34376, 0.35311, 0.36253, 0.37204, 0.38161, 0.39126,
  0.40096, 0.41072, 0.42054, 0.43040, 0.44030, 0.45023, 0.46020, 0.47019,
  0.48020, 0.49022, 0.50025, 0.51028, 0.52031, 0.53033, 0.54033, 0.55031,
  0.56026, 0.57019, 0.58007, 0.58991, 0.59970, 0.60944, 0.61912, 0.62873,
  0.63827, 0.64774, 0.65713, 0.66643, 0.67564, 0.68476, 0.69377, 0.70269,
  0.71150, 0.72019, 0.72877, 0.73723, 0.74557, 0.75378, 0.76186, 0.76981,
  0.77762, 0.78530, 0.79283, 0.80022, 0.80747, 0.81457, 0.82151, 0.82831,
  0.83496, 0.84145, 0.84779, 0.85398, 0.86001, 0.86588, 0.87160, 0.87716,
  0.88257, 0.88782, 0.89291, 0.89785, 0.90264, 0.90728, 0.91176, 0.91610,
  0.92028, 0.92432, 0.92822, 0.93197, 0.93558, 0.93906, 0.94240, 0.94560,
  0.94867, 0.95162, 0.95444, 0.95713, 0.95971, 0.96217, 0.96451, 0.96674,
  0.96887, 0.97089, 0.97281, 0.97463, 0.97635, 0.97799, 0.97953, 0.98099,
  0.98236, 0.98366, 0.98488, 0.98602, 0.98710, 0.98811, 0.98905, 0.98994,
  0.99076, 0.99153, 0.99225, 0.99291, 0.99353, 0.99411, 0.99464, 0.99513,
  0.99558, 0.99600, 0.99639, 0.99674, 0.99706, 0.99736, 0.99763, 0.99788,
  0.99811, 0.99831, 0.99850, 0.99867, 0.99882, 0.99895, 0.99908, 0.99919,
  0.99929, 0.99938, 0.99946, 0.99953, 0.99959, 0.99965, 0.99969, 0.99974,
  0.99978, 0.99981, 0.99984, 0.99986, 0.99988, 0.99990, 0.99992, 0.99993,
  0.99994, 0.99995, 0.99996, 0.99997, 0.99998, 0.99998, 0.99998, 0.99999,
  0.99999, 0.99999, 0.99999, 1.00000, 1.00000, 1.00000, 1.00000, 1.00000,
  1.00000, 1.00000, 1.00000, 1.00000, 1.00000, 1.00000, 1.00000, 1.00000
};

const uint8_t fftorder[] =
{
    0, 128,  64, 192,  32, 160, 224,  96,  16, 144,  80, 208, 240, 112,  48, 176,
    8, 136,  72, 200,  40, 168, 232, 104, 248, 120,  56, 184,  24, 152, 216,  88,
    4, 132,  68, 196,  36, 164, 228, 100,  20, 148,  84, 212, 244, 116,  52, 180,
  252, 124,  60, 188,  28, 156, 220,  92,  12, 140,  76, 204, 236, 108,  44, 172,
    2, 130,  66, 194,  34, 162, 226,  98,  18, 146,  82, 210, 242, 114,  50, 178,
   10, 138,  74, 202,  42, 170, 234, 106, 250, 122,  58, 186,  26, 154, 218,  90,
  254, 126,  62, 190,  30, 158, 222,  94,  14, 142,  78, 206, 238, 110,  46, 174,
    6, 134,  70, 198,  38, 166, 230, 102, 246, 118,  54, 182,  22, 150, 214,  86
};

IMDCT::IMDCT()
{
  int i, k;

  buf64_1 = buf128;
  buf64_2 = buf128 + 64;

  for (i = 0; i < 3; i++)
    roots16[i] = cos ((M_PI / 8) * (i + 1));

  for (i = 0; i < 7; i++)
    roots32[i] = cos ((M_PI / 16) * (i + 1));

  for (i = 0; i < 15; i++)
    roots64[i] = cos ((M_PI / 32) * (i + 1));

  for (i = 0; i < 31; i++)
    roots128[i] = cos ((M_PI / 64) * (i + 1));

  for (i = 0; i < 64; i++)
  {
    k = fftorder[i] / 2 + 64;
    pre1[i].real = cos ((M_PI / 256) * (k - 0.25));
    pre1[i].imag = sin ((M_PI / 256) * (k - 0.25));
  }

  for (i = 64; i < 128; i++)
  {
    k = fftorder[i] / 2 + 64;
    pre1[i].real = -cos ((M_PI / 256) * (k - 0.25));
    pre1[i].imag = -sin ((M_PI / 256) * (k - 0.25));
  }

  // Post-IFFT coefs are pre-scaled by 2
  // (moved here from overlap/add step)
  for (i = 0; i < 64; i++)
  {
    post1[i].real = 2 * cos ((M_PI / 256) * (i + 0.5));
    post1[i].imag = 2 * sin ((M_PI / 256) * (i + 0.5));
  }

  for (i = 0; i < 64; i++)
  {
    k = fftorder[i] / 4;
    pre2[i].real = cos ((M_PI / 128) * (k - 0.25));
    pre2[i].imag = sin ((M_PI / 128) * (k - 0.25));
  }

  // Post-IFFT coefs are pre-scaled by 2
  // (moved here from overlap/add step)
  for (i = 0; i < 32; i++)
  {
    post2[i].real = 2 * cos ((M_PI / 128) * (i + 0.5));
    post2[i].imag = 2 * sin ((M_PI / 128) * (i + 0.5));
  }

}

void IMDCT::ifft16(complex_t * buf)
{
  ifft8 (buf);
  ifft4 (buf + 8);
  ifft4 (buf + 12);
  ifft_pass (buf, roots16 - 4, 4);
}

void IMDCT::ifft32(complex_t *buf)
{
  ifft16 (buf);
  ifft8 (buf + 16);
  ifft8 (buf + 24);
  ifft_pass (buf, roots32 - 8, 8);
}

void IMDCT::ifft64(complex_t *buf)
{
  ifft32 (buf);
  ifft16 (buf + 32);
  ifft16 (buf + 48);
  ifft_pass (buf, roots64 - 16, 16);
}

void IMDCT::ifft128(complex_t *buf)
{
  ifft32 (buf);
  ifft16 (buf + 32);
  ifft16 (buf + 48);
  ifft_pass (buf, roots64 - 16, 16);

  ifft32 (buf + 64);
  ifft32 (buf + 96);
  ifft_pass (buf, roots128 - 32, 32);
}

void IMDCT::imdct_512(sample_t *data, sample_t *delay)
{
  int i, k;
  sample_t t_r, t_i, a_r, a_i, b_r, b_i, w_1, w_2;
  const sample_t *window = imdct_window;

  for (i = 0; i < 128; i++)
  {
    k = fftorder[i];
    t_r = pre1[i].real;
    t_i = pre1[i].imag;

    buf128[i].real = t_i * data[255-k] + t_r * data[k];
    buf128[i].imag = t_r * data[255-k] - t_i * data[k];
  }

  ifft128 (buf128);

  // Post IFFT complex multiply plus IFFT complex conjugate
  // Window and convert to real valued signal
  for (i = 0; i < 64; i++)
  {
    // y[n] = z[n] * (xcos1[n] + j * xsin1[n]);
    t_r = post1[i].real;
    t_i = post1[i].imag;

    a_r = t_r * buf128[i].real     + t_i * buf128[i].imag;
    a_i = t_i * buf128[i].real     - t_r * buf128[i].imag;
    b_r = t_i * buf128[127-i].real + t_r * buf128[127-i].imag;
    b_i = t_r * buf128[127-i].real - t_i * buf128[127-i].imag;

    w_1 = window[2*i];
    w_2 = window[255-2*i];
    data[2*i]     = delay[2*i] * w_2 - a_r * w_1;
    data[255-2*i] = delay[2*i] * w_1 + a_r * w_2;
    delay[2*i] = a_i;

    w_1 = window[2*i+1];
    w_2 = window[254-2*i];
    data[2*i+1]   = delay[2*i+1] * w_2 + b_r * w_1;
    data[254-2*i] = delay[2*i+1] * w_1 - b_r * w_2;
    delay[2*i+1] = b_i;
  }
}

void IMDCT::imdct_256(sample_t *data, sample_t *delay)
{
  int i, k;
  sample_t t_r, t_i, a_r, a_i, b_r, b_i, c_r, c_i, d_r, d_i, w_1, w_2;
  const sample_t * window = imdct_window;

  // Pre IFFT complex multiply plus IFFT complex conjugate
  for (i = 0; i < 64; i++) {
    k = fftorder[i];
    t_r = pre2[i].real;
    t_i = pre2[i].imag;

    buf64_1[i].real = t_i * data[254-k] + t_r * data[k];
    buf64_1[i].imag = t_r * data[254-k] - t_i * data[k];

    buf64_2[i].real = t_i * data[255-k] + t_r * data[k+1];
    buf64_2[i].imag = t_r * data[255-k] - t_i * data[k+1];
  }

  ifft64 (buf64_1);
  ifft64 (buf64_2);

  // Post IFFT complex multiply
  // Window and convert to real valued signal
  for (i = 0; i < 32; i++) {
    // y1[n] = z1[n] * (xcos2[n] + j * xs in2[n]) ;
    t_r = post2[i].real;
    t_i = post2[i].imag;

    a_r = t_r * buf64_1[i].real    + t_i * buf64_1[i].imag;
    a_i = t_i * buf64_1[i].real    - t_r * buf64_1[i].imag;
    b_r = t_i * buf64_1[63-i].real + t_r * buf64_1[63-i].imag;
    b_i = t_r * buf64_1[63-i].real - t_i * buf64_1[63-i].imag;

    c_r = t_r * buf64_2[i].real    + t_i * buf64_2[i].imag;
    c_i = t_i * buf64_2[i].real    - t_r * buf64_2[i].imag;
    d_r = t_i * buf64_2[63-i].real + t_r * buf64_2[63-i].imag;
    d_i = t_r * buf64_2[63-i].real - t_i * buf64_2[63-i].imag;

    w_1 = window[2*i];
    w_2 = window[255-2*i];
    data[2*i]     = delay[2*i] * w_2 - a_r * w_1;
    data[255-2*i] = delay[2*i] * w_1 + a_r * w_2;
    delay[2*i] = c_i;

    w_1 = window[128+2*i];
    w_2 = window[127-2*i];
    data[128+2*i] = delay[127-2*i] * w_2 + a_i * w_1;
    data[127-2*i] = delay[127-2*i] * w_1 - a_i * w_2 ;
    delay[127-2*i] = c_r;

    w_1 = window[2*i+1];
    w_2 = window[254-2*i];
    data[2*i+1]   = delay[2*i+1] * w_2 - b_i * w_1;
    data[254-2*i] = delay[2*i+1] * w_1 + b_i * w_2;
    delay[2*i+1] = d_r;

    w_1 = window[129+2*i];
    w_2 = window[126-2*i];
    data[129+2*i] = delay[126-2*i] * w_2 + b_r * w_1;
    data[126-2*i] = delay[126-2*i] * w_1 - b_r * w_2;
    delay[126-2*i] = d_i;
  }
}

