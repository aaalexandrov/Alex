#include "stdafx.h"
#include "Threads.h"

// CThreadBase ----------------------------------------------------------------
CRTTIRegisterer<CThreadBase> g_RegThreadBase;

// CThreadWin -----------------------------------------------------------------
CRTTIRegisterer<CThreadWin> g_RegThreadWin;

CThreadWin::CThreadWin()
{
  m_hThread = 0;
  m_dwThreadID = 0;
  m_pParam = 0;
}

CThreadWin::~CThreadWin()
{
  if (m_dwThreadID)
    Terminate(0);
  if (m_hThread)
    CloseHandle(m_hThread);
}

bool CThreadWin::Init(void *pParam, bool bCreateSuspended, UINT uiStackSize)
{
  m_pParam = pParam;
  m_hThread = CreateThread(0, uiStackSize, ThreadFunc, this, CREATE_SUSPENDED, &m_dwThreadID);
  ASSERT(m_dwThreadID);
  if (!m_dwThreadID) 
    return false;
  if (!bCreateSuspended)
    Resume();
  return true;
}

UINT CThreadWin::GetID()
{
  return m_dwThreadID;
}

int CThreadWin::Suspend()
{
  return SuspendThread(m_hThread);
}

int CThreadWin::Resume()
{
  return ResumeThread(m_hThread);
}

bool CThreadWin::Terminate(UINT uiExitcode)
{
  m_dwThreadID = 0;
  return !!TerminateThread(m_hThread, uiExitcode);
}

UINT CThreadWin::GetExitCode()
{
  DWORD dwExitcode;
  if (!GetExitCodeThread(m_hThread, &dwExitcode) || dwExitcode == STILL_ACTIVE)
    dwExitcode = -1;
  return dwExitcode;
}

UINT CThreadWin::Wait(UINT uiMilliseconds)
{
  uint32_t dwRes = WaitForSingleObject(m_hThread, uiMilliseconds);
  switch (dwRes) {
    case WAIT_OBJECT_0:
      dwRes = 0;
      break;
    case WAIT_TIMEOUT:
      dwRes = 1;
      break;
    case WAIT_FAILED:
      dwRes = -1;
      break;
    default:
      ASSERT(0);
      break;
  }
  return dwRes;
}

void CThreadWin::Exit(UINT uiExitcode)
{
  m_dwThreadID = 0;
  ExitThread(uiExitcode);
}

void CThreadWin::Yield(UINT uiMilliseconds)
{
  SleepEx(uiMilliseconds, false);
}

UINT CThreadWin::Run(void *pParam)
{
  return 0;
}

DWORD WINAPI CThreadWin::ThreadFunc(LPVOID lpParam)
{
  CThreadWin *pThread = (CThreadWin *) lpParam;
  UINT uiRes = pThread->Run(pThread->m_pParam);
  pThread->m_dwThreadID = 0;
  return uiRes;
}

// CLockBase ------------------------------------------------------------------
CRTTIRegisterer<CLockBase> g_RegLockBase;

// CLockWin -------------------------------------------------------------------
CRTTIRegisterer<CLockWin> g_RegLockWin;

CLockWin::CLockWin(UINT uiSpinCount)
{
  InitializeCriticalSectionEx(&m_CS, uiSpinCount, 0);
}

CLockWin::~CLockWin()
{
  DeleteCriticalSection(&m_CS);
}

void CLockWin::Lock()
{
  EnterCriticalSection(&m_CS);
}

bool CLockWin::TryLock()
{
  return !!TryEnterCriticalSection(&m_CS);
}

void CLockWin::Unlock()
{
  LeaveCriticalSection(&m_CS);
}

void CLockWin::SetSpinCount(UINT uiSpinCount)
{
  SetCriticalSectionSpinCount(&m_CS, uiSpinCount);
}
