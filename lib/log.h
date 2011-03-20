/*
  Logging class
*/

#ifndef VALIB_LOG_H
#define VALIB_LOG_H

#define LOG_SCREEN 1 // print log at screen
#define LOG_HEADER 2 // print timestamp header
#define LOG_STATUS 4 // show status information

#define MAX_LOG_LEVELS 128

#include <Ac3Filter/AutoFile.h>
#include "vtime.h"

class Log
{
public:
  Log(int flags = LOG_SCREEN | LOG_HEADER | LOG_STATUS, const char *log_file = 0, vtime_t period = 0.1);

  void openGroup(const char *msg, ...);
  int  closeGroup(int expected_errors = 0);

  int getLevel(void);
  int getErrors(void);
  vtime_t getTime(void);

  int getTotalErrors(void);
  vtime_t getTotalTime(void);

  void status(const char *msg, ...);
  void msg(const char *msg, ...);
  int err(const char *msg, ...);
  int err_close(const char *msg, ...);

protected:
  int level;
  int errors[MAX_LOG_LEVELS];
  vtime_t time[MAX_LOG_LEVELS];

  int flags;
  int istatus;
  vtime_t period;
  vtime_t tstatus;

  AutoFile f;

  void clearStatus(void);
  void printHeader(int _level);

};

#endif

