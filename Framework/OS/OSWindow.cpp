#include "stdafx.h"
#include "OSWindow.h"

// COSWindow ------------------------------------------------------------------

CRTTIRegisterer<COSWindow> g_RegOSWindow;

COSWindow *COSWindow::Create(CStartUp const &kStartUp, CCallback *pCallback, CStrAny sName, CRect<int> const *rc)
{
#if defined(WINDOWS)
  static CRTTI const *pRTTI = CRTTIHolder::Get()->Find("CWinOSWindow");
#endif
  ASSERT(pRTTI);
  if (!pRTTI) 
    return 0;

  COSWindow *pWindow = (COSWindow *) pRTTI->CreateInstance();
  if (!pWindow || !pWindow->Init(kStartUp, pCallback, sName, rc)) 
    SAFE_DELETE(pWindow);
  
  return pWindow;
}