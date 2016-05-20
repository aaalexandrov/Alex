#ifndef __THREADS_H
#define __THREADS_H

#include "Base.h"
#include "RTTI.h"
#include "Util.h"

#ifdef Yield
#undef Yield
#endif

// Threads
class CThreadBase: public CObject {
  DEFRTTI(CThreadBase, CObject, false)
public:
  CThreadBase()          {}
  virtual ~CThreadBase() {}

  virtual bool Init(void *pParam, bool bCreateSuspended = false, UINT uiStackSize = 0) = 0;

  virtual UINT GetID()                    = 0;

  virtual int Suspend()                   = 0; // Return suspend count after operation
  virtual int Resume()                    = 0;

  virtual bool Terminate(UINT uiExitcode) = 0; // Terminate should be called from outside the thread
  virtual UINT GetExitCode()              = 0;

  virtual UINT Wait(UINT uiMilliseconds)  = 0;

  virtual void Exit(UINT uiExitcode)      = 0; // Exit should only be called from the executing thread context
  virtual void Yield(UINT uiMilliseconds) = 0;
  virtual UINT Run(void *pParam)          = 0;
};

// Locks
class CLockBase: public CObject {
  DEFRTTI(CLockBase, CObject, false)
public:
  virtual void Lock() = 0;
  virtual bool TryLock() = 0;
  virtual void Unlock() = 0;

  virtual void SetSpinCount(UINT uiSpinCount) = 0;
};

class CScopeLock {
public:
  CLockBase *m_pLock;

  inline CScopeLock(CLockBase *pLock): m_pLock(pLock) { if (m_pLock) m_pLock->Lock(); }
  inline ~CScopeLock() { if (m_pLock) m_pLock->Unlock(); }
};

#include "Windows/WinThreads.h"
#include "Linux/LinThreads.h"

template <class T>
class CAtomic {
public:
  CLock m_kLock;
  T     m_Val;

  CAtomic() {}
  CAtomic(T val): m_Val(val) {}

  operator T() { CScopeLock kAutoLock(&m_kLock); return m_Val; }
  CAtomic &operator =(T val) { CScopeLock kAutoLock(&m_kLock); m_Val = val; return *this; }

  template <class A>
  CAtomic &operator +=(A diff) { CScopeLock kAutoLock(&m_kLock); m_Val += diff; return *this; }
  template <class A>
  CAtomic &operator -=(A diff) { CScopeLock kAutoLock(&m_kLock); m_Val -= diff; return *this; }

  T Exchange (T val) { CScopeLock kAutoLock(&m_kLock); Util::Swap(m_Val, val); return val; }
};


#endif
