#ifndef __INPUT_H
#define __INPUT_H

#include "List.h"
#include "Vector.h"
#include "Timing.h"
#include <Windows.h>

class CInput {
public:
  enum EEventType {
    ET_MOUSEMOVE,
    ET_MOUSEDOWN,
    ET_MOUSEUP,
    ET_MOUSEDOUBLECLICK,
    ET_MOUSEWHEEL,
    ET_KEYDOWN,
    ET_KEYUP,
    ET_KEYCHAR,
  };

  enum EInputModifier {
    IM_SHIFT = 1,
    IM_ALT = 2,
    IM_CONTROL = 4,
  };

  class CEvent {
  public:
    EEventType      m_eEvent;
    int             m_iKey;                  // Contains the delta in case of ET_MOUSEWHEEL
    UINT            m_uiModifiers;
    CVector<2, int> m_vPos;
    CTime           m_Time;

    CEvent(EEventType eEvent, int iKey, UINT uiModifiers, CTime kTime);
  };

  class CEventListener {
  public:
    virtual bool OnInputEvent(CEvent *pEvent) = 0;
  };

  struct TListenerInfo {
    CEventListener *pListener;
    UINT            uiPriority;

    explicit TListenerInfo(CEventListener *pListen, UINT uiPrio) { pListener = pListen; uiPriority = uiPrio; }
    static inline bool Eq(CEventListener *pListener, const TListenerInfo &li) { return pListener == li.pListener; }
  };

  typedef CList<TListenerInfo, TListenerInfo, TListenerInfo> TListenerList;

  static const unsigned int DEFAULT_PRIORITY = 1024;

public:
  static CInput *s_pInput;
  TListenerList m_lstListeners;
  CRect<int> m_rcClip;
  HWND m_hWnd;

  CInput(HWND hWnd);
  ~CInput();

  static CInput *Get() { return s_pInput; }

  UINT GetModifiersPressed();
  bool IsKeyPressed(int iKey);

  bool GetKeyboardFocus();
  void SetKeyboardFocus(bool bFocus);

  CVector<2, int> GetMousePos();
  void SetMousePos(CVector<2, int> vPos);

  bool GetMouseCapture();
  void SetMouseCapture(bool bCapture);

  const CRect<int> *GetMouseClip();
  void SetMouseClip(const CRect<int> *pClipRect);

  void SetEventListener(CEventListener *pListener, UINT uiPriority = DEFAULT_PRIORITY);
  void RemoveEventListener(CEventListener *pListener);

  void BroadcastEvent(CEvent *pEvent);

  bool InputWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  CEvent *MakeMouseEvent(EEventType eEvent, int iKey, WPARAM wParam, LPARAM lParam);
};

#endif