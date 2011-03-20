#include "thread.h"

Thread::Thread()
{
  f_terminate = 0;
  f_threadId = 0;
  f_thread = 0;
  f_suspended = true;
}

Thread::~Thread()
{
  if (f_thread)
    terminate(0);
}

DWORD WINAPI 
Thread::ThreadProc(LPVOID param)
{
  if (param)
  {
    Thread *thread = (Thread *)param;
    DWORD exit_code = thread->process();
    CloseHandle(thread->f_thread);
    thread->f_thread = 0;
    return exit_code;
  }
  return 0;
}

bool
Thread::create(bool suspended)
{
  if (f_thread)
    terminate(0);

  f_terminate = 0;
  f_threadId = 0;
  f_thread = CreateThread(0, 0, ThreadProc, this, suspended? CREATE_SUSPENDED :0, &f_threadId);
  f_suspended = f_thread? f_suspended: true;
  return f_thread != 0;
}

void
Thread::suspend()
{
  if (f_thread && SuspendThread(f_thread) != -1)
    f_suspended = true;
}

void
Thread::resume()
{
  if (f_thread && ResumeThread(f_thread) != -1)
    f_suspended = false;
}

void 
Thread::terminate(int timeout_ms, DWORD exit_code)
{
  if (!f_thread) return;

  // wait for thread to finish correctly
  if (timeout_ms > 0)
  {
    f_terminate = true;
    WaitForSingleObject(f_thread, timeout_ms);
  }

  // terminate it if it was not finished
  if (f_thread)
  {
    TerminateThread(f_thread, exit_code);
    CloseHandle(f_thread);
    f_thread = 0;
  }
  return;
}
