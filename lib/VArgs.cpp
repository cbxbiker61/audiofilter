#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <AudioFilter/Defs.h>
#include <AudioFilter/VArgs.h>

namespace AudioFilter {

bool isArg(char *arg, const char *name, arg_type type)
{
  if ( arg[0] != '-' )
    return false;

  ++arg;

  while ( *name )
  {
    if ( *name && *arg != *name )
      return false;
    else
    {
      ++name;
	  ++arg;
    }
  }

  if ( type == argt_exist && *arg == '\0' )
    return true;
  else if ( type == argt_bool && (*arg == '\0' || *arg == '+' || *arg == '-') )
    return true;
  else if ( type == argt_num && (*arg == ':' || *arg == '=') )
    return true;
  else if ( type == argt_hex && (*arg == ':' || *arg == '=') )
    return true;

  return false;
}

bool getBoolArg(char *arg)
{
  arg += strlen(arg) - 1;

  return *arg != '-';
}

double getNumArg(char *arg)
{
  arg += strlen(arg);

  while ( *arg != ':' && *arg != '=' )
    --arg;

  ++arg;
  return atof(arg);
}

int getHexArg(char *arg)
{
  arg += strlen(arg);

  while ( *arg != ':' && *arg != '=' )
    --arg;

  ++arg;

  int result;
  sscanf(arg, "%x", &result);
  return result;
}

}; // namespace AudioFilter

// vim: ts=2 sts=2 et

