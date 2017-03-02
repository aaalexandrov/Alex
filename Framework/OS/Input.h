#ifndef __INPUT_H
#define __INPUT_H

#include "../Base/List.h"
#include "../Base/Vector.h"
#include "Timing.h"
#include "../Base/Rect.h"

class COSWindow;

class CInput: public CObject {
  DEFRTTI(CInput, CObject, false)
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

  enum EKeyCode {
    // Special codes. All lower row capitalized symbols on a standard US keyboard are also valid key IDs
    KC_INVALID = 0,
    KC_ESCAPE = 27,
    KC_LSHIFT = 14,
    KC_LCTRL = 2,
    KC_LALT = 3,
    KC_LWIN = 4,
    KC_CAPSLOCK = 5,
    KC_RSHIFT = 15,
    KC_RCTRL = 7,
    KC_RALT = 11,
    KC_RWIN = 12,
    KC_INS = 1,
    KC_DEL = 6,
    KC_HOME = 16,
    KC_END = 17,
    KC_PAGEUP = 18,
    KC_PAGEDOWN = 19,
    KC_SCROLLLOCK = 20,
    KC_NUMLOCK = 21,
    KC_PRINTSCREEN = 22,
    KC_PAUSE = 23,
    KC_UP = 24,
    KC_DOWN = 25,
    KC_LEFT = 26,
    KC_RIGHT = 28,
    KC_102 = 'a',
    KC_F1,
    KC_F2,
    KC_F3,
    KC_F4,
    KC_F5,
    KC_F6,
    KC_F7,
    KC_F8,
    KC_F9,
    KC_F10,
    KC_F11,
    KC_F12,
    KC_LMOUSE,
    KC_RMOUSE,
    KC_MMOUSE,
    KC_MOUSE4,
    KC_MOUSE5,

    KC_NUMPAD = 0x80,
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
  CTimer m_kTimer;
  COSWindow *m_pWindow;

  static CInput *Get() { return s_pInput; }

  static void Create(COSWindow *pWindow);
  static void Destroy();

  CInput();
  virtual ~CInput();

  virtual COSWindow *GetOSWindow() { return m_pWindow; }
  virtual void SetOSWindow(COSWindow *pWindow) { m_pWindow = pWindow; }

  virtual UINT GetModifiersPressed() = 0;
  virtual bool IsKeyPressed(int iKey) = 0;

  virtual bool GetKeyboardFocus() = 0;
  virtual void SetKeyboardFocus(bool bFocus) = 0;

  virtual CVector<2, int> GetMousePos() = 0;
  virtual void SetMousePos(CVector<2, int> vPos) = 0;

  virtual bool GetMouseCapture() = 0;
  virtual void SetMouseCapture(bool bCapture) = 0;

  virtual const CRect<int> *GetMouseClip();
  virtual void SetMouseClip(const CRect<int> *pClipRect);

  virtual void SetEventListener(CEventListener *pListener, UINT uiPriority = DEFAULT_PRIORITY);
  virtual void RemoveEventListener(CEventListener *pListener);

  virtual void BroadcastEvent(CEvent *pEvent);
};

#endif
