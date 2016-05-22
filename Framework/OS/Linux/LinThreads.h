#ifndef __LINTHREADS_H
#define __LINTHREADS_H

#ifdef LINUX

#include "Threads.h"
#include <pthread.h>

class CThreadLin: public CThreadBase {
  DEFRTTI(CThreadLin, CThreadBase, true)
public:
  pthread_t  m_Thread;
  void      *m_pParam;
  uintptr_t  m_uiExitCode;

  CThreadLin();
  virtual ~CThreadLin();

  virtual bool Init(void *pParam, bool bCreateSuspended = false, UINT uiStackSize = 0);

  virtual uintptr_t GetID();

  virtual int Suspend(); // Return suspend count after operation
  virtual int Resume();

  virtual bool Terminate(uintptr_t uiExitcode); // Terminate should be called from outside the thread
  virtual uintptr_t GetExitCode();

  virtual UINT Wait(UINT uiMilliseconds);

  virtual void Exit(uintptr_t uiExitcode); // Exit should only be called from the executing thread context
  virtual void Yield(UINT uiMilliseconds);
  virtual uintptr_t Run(void *pParam);

  static __cdecl void *ThreadFunc(void *pParam);
};

typedef CThreadLin CThread;

// Locks
class CLockLin: public CLockBase {
  DEFRTTI(CLockLin, CLockBase, true)
public:
  pthread_mutex_t m_Mutex;

  CLockLin();
  virtual ~CLockLin();

  virtual void Lock();
  virtual bool TryLock();
  virtual void Unlock();

  virtual void SetSpinCount(UINT uiSpinCount);
};

typedef CLockLin CLock;

#endif // LINUX

#endif // __LINTHREADS_H
