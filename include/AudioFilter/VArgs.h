#pragma once
#ifndef AUDIOFILTER_VARGS_H
#define AUDIOFILTER_VARGS_H
/*
  Command-line arguments handling
*/

namespace AudioFilter {

enum arg_type { argt_exist, argt_bool, argt_num, argt_hex };
bool isArg(char *arg, const char *name, arg_type type);
bool getBoolArg(char *arg);
double getNumArg(char *arg);
int getHexArg(char *arg);

}; // namespace AudioFilter

#endif

// vim: ts=2 sts=2 et

