/*
  CPU usage measurement
*/

#ifndef VALIB_CPU_H
#define VALIB_CPU_H

#include <windows.h>
#include "../vtime.h"

class CPUMeter
{
private:
  int      ncpus;             // number of processors
  HANDLE   thread;            // monitored thread handle copy (can be used by other threads)
  __int64  thread_time;       // thread time spent in between of usage() calls
  __int64  system_time_begin; // system time of previous usage() call
  __int64  thread_time_begin; // thread time we start measure
  __int64  thread_time_total; // total thread time spent

  __int64  system_time_start; // time we start measure
  __int64  system_time_total; // total system time spent in between of start() and stop() calls

public:
  CPUMeter();

  // methods to be called by thread measured
  void    start();  // start measurement
  void    stop();   // stop measurement

  // methods to be called by monitor thread (may be other thread than thread measured)
  // can be called at any time, including the time in between start() and stop() calls

  void    reset();  // reset counters
  double  usage();  // mean CPU usage since last usage() call (only thread time spent in between start() and stop() 
                    // calls is counted; this call resets time counters)

  vtime_t get_thread_time();    // time used by thread since last reset() or usage() call
  vtime_t get_system_time();    // system time spent since last reset() or usage() call
  int     get_number_of_cpus(); // number of processors

  // mean CPU usage since last reset() or usage() call (only thread time spent in between start() and stop()
  // calls is counted; this call does not reset time counters)
  double  mean_usage() { return get_thread_time() / get_system_time(); };
};

#endif
