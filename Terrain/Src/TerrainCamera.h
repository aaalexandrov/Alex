#ifndef __TERRAINCAMERA_H
#define __TERRAINCAMERA_H

#include "Camera.h"
#include "Input.h"

class CFreeCamera: public CInput::CEventListener {
public:
  struct TKeyInfo {
    int   iKey;
    CTime kPressedTime;
    bool  bPressed: 1, 
          bPressConsumed: 1;

    explicit TKeyInfo(int iK): kPressedTime(0)    { iKey = iK; bPressed = false; bPressConsumed = false; }
    static inline bool Eq(int iKey, TKeyInfo &ki) { return iKey == ki.iKey; }
    static inline size_t Hash(const TKeyInfo &ki) { return ki.iKey; }
    static inline size_t Hash(int i) { return i; }
  };

  typedef CHash<TKeyInfo, int, TKeyInfo, TKeyInfo> TKeyHash;

public:
  CCamera *m_pCamera;
  bool m_bOwnCamera;
  TKeyHash m_hashKeys;
  bool m_bFreeMode, m_bWalkMode;
  CVector<2, int> m_vMouseNow, m_vMouseLast;

  CFreeCamera(CCamera *pCam, bool bOwnCam);
  ~CFreeCamera();

  float GetKeyPressedSeconds(int iKey, CTime kNow);
  bool HasBeenPressed(int iKey);

  void Update(CTime kTime);

  virtual bool OnInputEvent(CInput::CEvent *pEvent);
};

#endif