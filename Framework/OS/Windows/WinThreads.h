#ifndef __WINTHREADS_H
#define __WINTHREADS_H

#ifdef WINDOWS

#include "Threads.h"
#include <Windows.h>

#ifdef Yield
#undef Yield
#endif

class CThreadWin: public CThreadBase {
  DEFRTTI(CThreadWin, CThreadBase, true)
public:
  HANDLE    m_hThread;
  DWORD     m_dwThreadID;
  void     *m_pParam;

  CThreadWin();
  virtual ~CThreadWin();

  virtual bool Init(void *pParam, bool bCreateSuspended = false, UINT uiStackSize = 0);

  virtual UINT GetID();

  virtual int Suspend();
  virtual int Resume();

  virtual bool Terminate(UINT uiExitcode);
  virtual UINT GetExitCode();

  virtual UINT Wait(UINT uiMilliseconds);

  virtual void Exit(UINT uiExitcode);
  virtual void Yield(UINT uiMilliseconds);
  virtual UINT Run(void *pParam);

  static DWORD WINAPI ThreadFunc(LPVOID lpParam);
};

typedef CThreadWin CThread;

class CLockWin: public CLockBase {
  DEFRTTI(CLockWin, CLockBase, true)
public:
  CRITICAL_SECTION m_CS;

  CLockWin(UINT uiSpinCount = 0);
  virtual ~CLockWin();

  virtual void Lock();
  virtual bool TryLock();
  virtual void Unlock();

  virtual void SetSpinCount(UINT uiSpinCount);
};

typedef CLockWin CLock;

#endif // WINDOWS

#endif // __WINTHREADS_H