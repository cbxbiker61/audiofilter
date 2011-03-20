#include <stdio.h>
#include "defs.h"

///////////////////////////////////////////////////////////////////////////////
// matrix_t

matrix_t &
matrix_t::operator =(const matrix_t &m)
{
  for (int i = 0; i < NCHANNELS; i++)
    for (int j = 0; j < NCHANNELS; j++)
      matrix[i][j] = m[i][j];
  return *this;
}

matrix_t &
matrix_t::zero()
{
  for (int i = 0; i < NCHANNELS; i++)
    for (int j = 0; j < NCHANNELS; j++)
      matrix[i][j] = 0;
  return *this;
}

matrix_t &
matrix_t::identity()
{
  zero();
  for (int i = 0; i < NCHANNELS; i++)
    matrix[i][i] = 1.0;
  return *this;
}

bool
matrix_t::operator ==(const matrix_t &m) const
{
  for (int i = 0; i < NCHANNELS; i++)
    for (int j = 0; j < NCHANNELS; j++)
      if (matrix[i][j] != m.matrix[i][j])
        return false;
  return true;
}

bool
matrix_t::operator !=(const matrix_t &m) const
{ return !(*this == m); }

///////////////////////////////////////////////////////////////////////////////
// Build info

static char info_str[1024];

const char *valib_build_info()
{
  static bool init = false;
  if (!init)
  {
    int ptr = 0;
    ptr += sprintf(info_str + ptr, "Compiler: "
    #if defined(_MSC_VER)
      "MSVC %i\n", _MSC_VER);
    #elif defined(__GNUC__)
      "GCC %s\n", __VERSION__);
    #else
      "Unknown%s\n",
    #endif

    ptr += sprintf(info_str + ptr, "Debug/Release: "
    #ifdef _DEBUG
      "Debug\n");
    #else
      "Release\n");
    #endif

    ptr += sprintf(info_str + ptr, "Build date: " __DATE__ " " __TIME__"\n");
    ptr += sprintf(info_str + ptr, "Number of channels: %i\n", NCHANNELS);
    ptr += sprintf(info_str + ptr, "Sample format: "
    #ifdef FLOAT_SAMPLE
      "float\n");
    #else
      "double\n");
    #endif
    ptr += sprintf(info_str + ptr, "Sample size: %i\n", sizeof(sample_t));
    init = true;
  }

  return info_str;
}

const char *valib_credits()
{
  return
    "libca (former libdts) - DTS decoder library\n"
    "http://developers.videolan.org/libdca.html\n"
    "\n"
    "ffmpeg - AC3 encoder\n"
    "http://ffmpeg.mplayerhq.hu\n"
    "\n"
    "FFT and Bessel code by Takuya OOURA\n"
    "http://www.kurims.kyoto-u.ac.jp/~ooura\n"
    "\n"
    "muxman - DVD authoring\n"
    "http://www.mpucoder.com/Muxman\n"
    "\n"
    "Media Player Classic - best media player\n"
    "http://sourceforge.net/projects/guliverkli\n";
}
