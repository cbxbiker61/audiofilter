#include <AudioFilter/VTime.h>

namespace AudioFilter {

///////////////////////////////////////////////////////////
// Win32 implementation

#ifdef _WIN32

#include <windows.h>

// constant is number of 100ns intervals between
// January 1, 1601 and January 1, 1970.

#if defined(__GNUC__)
static const __int64 epoch_adj = 0x019db1ded53e8000LL;
#else
static const __int64 epoch_adj = 0x019db1ded53e8000;
#endif

vtime_t getUtcTime(void)
{
  __int64 ft;
  SYSTEMTIME st;
  GetSystemTime(&st);
  SystemTimeToFileTime(&st,(FILETIME*)&ft);
  return vtime_t(ft - epoch_adj) / 10000000; // 10Mhz clock
}

vtime_t getLocalTime(void)
{
  __int64 ft;
  SYSTEMTIME st;

  GetLocalTime(&st);
  SystemTimeToFileTime(&st,(FILETIME*)&ft);
  vtime_t result = vtime_t(ft - epoch_adj) / 10000000; // 10Mhz clock
  return result;
}

vtime_t toLocalTime(vtime_t _time)
{
  __int64 utc = (__int64)(_time * 10000000 + epoch_adj);
  __int64 local;

  FileTimeToLocalFileTime((FILETIME*)&utc, (FILETIME*)&local);
  return vtime_t(local - epoch_adj) / 10000000;
}

vtime_t toUtcTime(vtime_t _time)
{
  __int64 local = (__int64)(_time * 10000000 + epoch_adj);
  __int64 utc;

  LocalFileTimeToFileTime((FILETIME*)&local, (FILETIME*)&utc);
  return vtime_t(utc - epoch_adj) / 10000000;
}

///////////////////////////////////////////////////////////
// Time.h implementation

#elif __GNUC__

#include <time.h>

vtime_t getLocalTime(void)
{
  time_t t;
  return (vtime_t)time(&t);
}

#else

#error "No implementations for time functions"

#endif

}; // namespace AudioFilter

// vim: ts=2 sts=2 et

