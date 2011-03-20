#include <windows.h>
#include <stdio.h>

__int64 get_process_time(HANDLE process)
{
  __int64 creation_time;
  __int64 exit_time;
  __int64 kernel_time;
  __int64 user_time;

  if (GetProcessTimes(process, 
         (FILETIME*)&creation_time, 
         (FILETIME*)&exit_time, 
         (FILETIME*)&kernel_time, 
         (FILETIME*)&user_time))
    return kernel_time + user_time;
  return -1;
}

__int64 get_system_time()
{
  __int64 time;
  SYSTEMTIME systime;
  GetSystemTime(&systime);
  SystemTimeToFileTime(&systime, (FILETIME*)&time);
  return time;
}

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    printf(
"cpu_meter\n"
"=========\n"
"Measure the CPU time consumed by a program run.\n"
"\n"
"This utility is a part of AC3Filter project (http://ac3filter.net)\n"
"Copyright (c) 2008-2009 by Alexander Vigovsky\n"
"\n"
"Usage:\n"
"  > cpu_meter program [arg1 [arg2 [...]]\n"
    );
    return 0;
  }

  LPTSTR command_line = GetCommandLine();
  while (command_line[0] && command_line[0] != ' ' && command_line[0] != '\t') command_line++;
  while (command_line[0] == ' ' || command_line[0] == '\t') command_line++;

  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  memset(&si, 0, sizeof(si));
  si.cb = sizeof(si);

  if (CreateProcess(0, command_line, 0, 0, TRUE, CREATE_SUSPENDED, 0, 0, &si, &pi) == 0)
  {
    printf("Cannot start the program\n");
    return -1;
  }
  WaitForInputIdle(pi.hProcess, INFINITE);

  __int64 process_time = get_process_time(pi.hProcess);
  __int64 system_time = get_system_time();

  ResumeThread(pi.hThread);
  WaitForSingleObject(pi.hProcess, INFINITE);

  process_time = get_process_time(pi.hProcess) - process_time;
  system_time = get_system_time() - system_time;

  printf("--------------------\n");
  printf("Process time: %ims\n", int(process_time / 10000));
  printf("System time: %ims\n", int(system_time / 10000));
  return int(process_time / 10000);
}
