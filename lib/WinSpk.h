/*
  Windows-related speakers utilities
*/

#ifndef VALIB_WINSPK_H
#define VALIB_WINSPK_H

#include <AudioFilter/Speakers.h>

#ifdef _WIN32
#include <windows.h>
#include <ks.h>
#include <ksmedia.h>
#else

class GUID
{
public:
  uint32_t Data1;
  uint16_t Data2;
  uint16_t Data3;
  uint8_t  Data4[8];

  bool operator == (const GUID &o)
  {
    return Data1 == o.Data1
      && Data2 == o.Data2
      && Data3 == o.Data3
      && Data4[0] == o.Data4[0]
      && Data4[1] == o.Data4[1]
      && Data4[2] == o.Data4[2]
      && Data4[3] == o.Data4[3]
      && Data4[4] == o.Data4[4]
      && Data4[5] == o.Data4[5]
      && Data4[6] == o.Data4[6]
      && Data4[7] == o.Data4[7];
  }
};

typedef struct
{
  uint16_t wFormatTag;
  uint16_t nChannels;
  uint32_t nSamplesPerSec;
  uint32_t nAvgBytesPerSec;
  uint16_t nBlockAlign;
  uint16_t wBitsPerSample;
  uint16_t cbSize;
} WAVEFORMATEX, *PWAVEFORMATEX;

typedef struct {
  WAVEFORMATEX Format;
  union {
    uint16_t wValidBitsPerSample;
    uint16_t wSamplesPerBlock;
    uint16_t wReserved;
  } Samples;
  uint32_t dwChannelMask;
  GUID     SubFormat;
} WAVEFORMATEXTENSIBLE, *PWAVEFORMATEXTENSIBLE;

#endif

#ifndef WAVE_FORMAT_PCM
#define WAVE_FORMAT_PCM 1
#endif

#define WAVE_FORMAT_IEEE_FLOAT 0x0003
#define WAVE_FORMAT_EXTENSIBLE 0xFFFE
#define WAVE_FORMAT_MPEG 0x0050
#define WAVE_FORMAT_AVI_AC3 0x2000
#define WAVE_FORMAT_AVI_DTS 0x2001
#define WAVE_FORMAT_DOLBY_AC3_SPDIF 0x0092

namespace AudioFilter {

bool wfx2spk(WAVEFORMATEX *wfx, Speakers &spk);
bool spk2wfx(Speakers spk, WAVEFORMATEX *wfx, bool use_extensible);
bool is_compatible(Speakers spk, WAVEFORMATEX *wfx);

}; // namespace AudioFilter

#endif

// vim: ts=2 sts=2 et

