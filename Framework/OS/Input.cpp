#include "stdafx.h"
#include "Input.h"
#include "Debug.h"

// CInput::CEvent -------------------------------------------------------------

CInput::CEvent::CEvent(EEventType eEvent, int iKey, UINT uiModifiers, CTime kTime)
: m_Time(kTime)
{
  m_eEvent = eEvent;
  m_iKey = iKey;
  m_uiModifiers = uiModifiers;
}


// CInput ---------------------------------------------------------------------

CRTTIRegisterer<CInput> g_RegInput;

CInput *CInput::s_pInput = 0;

void CInput::Create(COSWindow *pWindow)
{
#if defined(WINDOWS)
  char const *chInputClass = "CWinInput";
#endif
  CRTTIHolder::Get()->Find(chInputClass)->CreateInstance();
  CInput::Get()->SetOSWindow(pWindow);
}

void CInput::Destroy() 
{
  delete CInput::Get();
}

CInput::CInput()
{
  ASSERT(!s_pInput);
  s_pInput = this;
  m_kTimer.Init();
  m_pWindow = 0;
  m_rcClip.SetEmpty();
}

CInput::~CInput()
{
  ASSERT(s_pInput == this);
  s_pInput = 0;
}

const CRect<int> *CInput::GetMouseClip()
{
  return &m_rcClip;
}

void CInput::SetMouseClip(const CRect<int> *pClipRect)
{
  if  (pClipRect) 
    m_rcClip = *pClipRect;
  else
    m_rcClip.SetEmpty();
}

void CInput::SetEventListener(CEventListener *pListener, UINT uiPriority)
{
  RemoveEventListener(pListener);
  TListenerList::TNode *pNode;
  for (pNode = m_lstListeners.m_pHead; pNode; pNode = pNode->pNext)
    if (pNode->Data.uiPriority < uiPriority)
      m_lstListeners.PushBefore(pNode, TListenerInfo(pListener, uiPriority));
  if (!pNode)
    m_lstListeners.PushTail(TListenerInfo(pListener, uiPriority));
}

void CInput::RemoveEventListener(CEventListener *pListener)
{
  m_lstListeners.Remove(m_lstListeners.Find(pListener));
}

void CInput::BroadcastEvent(CEvent *pEvent)
{
  TListenerList::TNode *pNode;
  for (pNode = m_lstListeners.m_pHead; pNode; pNode = pNode->pNext) {
    bool bContinue = pNode->Data.pListener->OnInputEvent(pEvent);
    if (!bContinue)
      break;
  }
  delete pEvent;
}

