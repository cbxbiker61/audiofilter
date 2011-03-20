#include <stdarg.h>
#include <time.h>
#include "log.h"


static const char *statuses[] = { "* ", "- ", "\\ ", "| ", "/ " };

inline void log_print(int flags, AutoFile &f, const char *format, va_list list)
{
  if (flags & LOG_SCREEN)
    vprintf(format, list);
  if (f.is_open())
  {
    vfprintf(f, format, list);
    fflush(f);
  }
}

inline void log_print(int flags, AutoFile &f, const char *format, ...)
{
  va_list list;
  va_start(list, format);
  log_print(flags, f, format, list);
  va_end(list);
}


Log::Log(int _flags, const char *_log_file, vtime_t _period)
{
  level = 0;
  errors[0] = 0;
  time[0] = local_time();
 
  flags = _flags;
  istatus = 0;
  period = _period;
  tstatus = local_time();

  if (_log_file)
    if (!f.open(_log_file, "w"))
      msg("Cannot open log file %s", _log_file);
}

void
Log::clear_status()
{
  // erase status line (if it is)
  if (istatus)
  {
    printf("                                                                               \r");
    istatus = 0;
  }
}

void 
Log::print_header(int _level)
{
  clear_status();

  if (flags & LOG_HEADER)
  {
    // timestamp
    time_t t = (time_t)local_time();
    tm *pt = gmtime(&t);
    if (pt)
      log_print(flags, f, "%04i/%02i/%02i %02i:%02i:%02i | ", 
          pt->tm_year + 1900, pt->tm_mon + 1, pt->tm_mday, 
          pt->tm_hour, pt->tm_min, pt->tm_sec);
    else
      log_print(flags, f, "0000/00/00 00:00:00 | ");
  }
  
  // indent
  while (_level--)
    log_print(flags, f, "  ");
}



void 
Log::open_group(const char *msg, ...)
{
  print_header(level);

  log_print(flags, f, "> ");
  va_list list;
  va_start(list, msg);
  log_print(flags, f, msg, list);
  va_end(list);
  log_print(flags, f, "\n");

  if (level < MAX_LOG_LEVELS)
  {
    level++;
    errors[level] = 0;
    time[level] = local_time();
  }
}

int 
Log::close_group(int _expected_errors)
{
  print_header(level? level-1: 0);

  vtime_t elapsed = local_time() - time[level];

  if (!errors[level] && !_expected_errors)
    log_print(flags, f, "< Ok. (%i:%02i)\n", 
      int(elapsed) / 60, int(elapsed) % 60);
  else if (errors[level] == _expected_errors)
    log_print(flags, f, "< Ok. Expected errors: %i (%i:%02i)\n", 
      _expected_errors, int(elapsed) / 60, int(elapsed) % 60);
  else if (!_expected_errors)
    log_print(flags, f, "< Fail. Errors: %i (%i:%02i)\n", 
      errors[level], int(elapsed) / 60, int(elapsed) % 60);
  else
    log_print(flags, f, "< Fail. Errors/expected errors: %i/%i (%i:%02i)\n", 
      errors[level], _expected_errors, int(elapsed) / 60, int(elapsed) % 60);

  if (errors[level] > _expected_errors)
    errors[level] = errors[level] - _expected_errors;
  else
    errors[level] = _expected_errors - errors[level];

  if (level)
  {
    errors[level-1] += errors[level];
    level--;
    return errors[level+1];
  }
  else
    return errors[0];
}

int
Log::get_level() 
{ 
  return level; 
}

int 
Log::get_errors()
{
  return errors[level]; 
}

vtime_t
Log::get_time()
{
  return local_time() - time[level];
}

int 
Log::get_total_errors()
{ 
  int result = 0;
  for (int i = 0; i <= level; i++)
    result += errors[level];

  return result;
}

vtime_t
Log::get_total_time()
{
  return local_time() - time[0];
}


void 
Log::status(const char *_msg, ...)
{
  if (flags & LOG_STATUS)
  {
    vtime_t t = local_time();
    if (t > tstatus + period)
    {
      tstatus = t;
      istatus++;
      if (istatus >= (sizeof(statuses) / sizeof(statuses[0])))
        istatus = 1;
      fprintf(stderr, statuses[istatus]);
  
      va_list list;
      va_start(list, _msg);
      vfprintf(stderr, _msg, list);
      va_end(list);
      fprintf(stderr, "\r");
    }
  }
}

void 
Log::msg(const char *_msg, ...)
{
  print_header(level);

  log_print(flags, f, "* ");
  va_list list;
  va_start(list, _msg);
  log_print(flags, f, _msg, list);
  va_end(list);
  log_print(flags, f, "\n");
}

int 
Log::err(const char *_msg, ...)
{
  print_header(level);

  log_print(flags, f, "! error: ");
  va_list list;
  va_start(list, _msg);
  log_print(flags, f, _msg, list);
  va_end(list);
  log_print(flags, f, "\n");

  errors[level]++;
  return errors[level];
}

int 
Log::err_close(const char *_msg, ...)
{
  print_header(level);

  log_print(flags, f, "! error: ");
  va_list list;
  va_start(list, _msg);
  log_print(flags, f, _msg, list);
  va_end(list);
  log_print(flags, f, "\n");

  errors[level]++;
  return close_group();
}
