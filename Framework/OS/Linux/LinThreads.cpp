#include "stdafx.h"
#include "LinThreads.h"

#ifdef LINUX

#include <limits.h>

// CThreadLin ----------------------------------------------------------------
CRTTIRegisterer<CThreadLin> g_RegThreadLin;

CThreadLin::CThreadLin() : m_Thread(0), m_pParam(0), m_uiExitCode(-1)
{
}

CThreadLin::~CThreadLin()
{
  if (m_Thread)
    Terminate(-1);
}

bool CThreadLin::Init(void *pParam, bool bCreateSuspended, UINT uiStackSize)
{
  ASSERT(!m_Thread);
  ASSERT(!bCreateSuspended && "Can't suspend in pthreads");
  m_pParam = pParam;
  m_uiExitCode = -1;
  pthread_attr_t kAttr;
  pthread_attr_init(&kAttr);
  pthread_attr_setdetachstate(&kAttr, false);
  if (uiStackSize != 0)
    pthread_attr_setstacksize(&kAttr, Util::Max<size_t>(uiStackSize, PTHREAD_STACK_MIN));
  pthread_create(&m_Thread, &kAttr, ThreadFunc, this);
  pthread_attr_destroy(&kAttr);
  return true;
}

uintptr_t CThreadLin::GetID()
{
  return (uintptr_t) m_Thread;
}

int CThreadLin::Suspend()
{
  ASSERT(!"Can't suspend in pthreads");
  return -1;
}

int CThreadLin::Resume()
{
  ASSERT(!"Can't resume in pthreads");
  return -1;
}

bool CThreadLin::Terminate(uintptr_t uiExitcode)
{
  bool bSuccess = !pthread_cancel(m_Thread);
  if (bSuccess)
    m_uiExitCode = uiExitcode;
  return bSuccess;
}

uintptr_t CThreadLin::GetExitCode()
{
  return m_uiExitCode;
}

UINT CThreadLin::Wait(UINT uiMilliseconds)
{
  int iResult;
  if (uiMilliseconds) {
    timespec kTime;
    kTime.tv_sec = uiMilliseconds / 1000;
    kTime.tv_nsec = (uiMilliseconds % 1000) * 1000000;
    iResult = pthread_timedjoin_np(m_Thread, (void **) &m_uiExitCode, &kTime);
  } else
    iResult = pthread_join(m_Thread, (void **) &m_uiExitCode);
  return iResult;
}

void CThreadLin::Exit(uintptr_t uiExitcode)
{
  m_uiExitCode = uiExitcode;
  pthread_exit((void *) uiExitcode);
}

void CThreadLin::Yield(UINT uiMilliseconds)
{
  pthread_yield();
}

uintptr_t CThreadLin::Run(void *pParam)
{
  return 0;
}

void *CThreadLin::ThreadFunc(void *pParam)
{
  CThreadLin *pThread = (CThreadLin *) pParam;
  pThread->m_uiExitCode = pThread->Run(pThread->m_pParam);
  return (void *) pThread->m_uiExitCode;
}

// CLockLin ------------------------------------------------------------------
CRTTIRegisterer<CLockLin> g_RegLockLin;

CLockLin::CLockLin() : m_Mutex(PTHREAD_MUTEX_INITIALIZER)
{
  pthread_mutexattr_t kAttr;
  pthread_mutexattr_init(&kAttr);
  pthread_mutexattr_settype(&kAttr, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(&m_Mutex, &kAttr);
  pthread_mutexattr_destroy(&kAttr);
}

CLockLin::~CLockLin()
{
  pthread_mutex_destroy(&m_Mutex);
}

void CLockLin::Lock()
{
  pthread_mutex_lock(&m_Mutex);
}

bool CLockLin::TryLock()
{
  return !!pthread_mutex_trylock(&m_Mutex);
}

void CLockLin::Unlock()
{
  pthread_mutex_unlock(&m_Mutex);
}

void CLockLin::SetSpinCount(UINT uiSpinCount)
{
  ASSERT(!"Setting lock spin count unsupported on pthread");
}

#endif // LINUX
