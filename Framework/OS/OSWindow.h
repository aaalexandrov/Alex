#ifndef __OSWINDOW_H
#define __OSWINDOW_H

#include "RTTI.h"
#include "Rect.h"

class CStartUp;
class COSWindow: public CObject {
  DEFRTTI(COSWindow, CObject, false)
public:
  class CCallback {
  public:
    virtual void OnMoved(COSWindow &kWindow, CRect<int> const &rcOld)  {}
    virtual bool OnDraw(COSWindow &kWindow, CRect<int> const &rcDirty) { return false; } // Returns true if the rect was actually drawn by the method
  };

public:
  CCallback *m_pCallback;

  static COSWindow *Create(CStartUp const &kStartUp, CCallback *pCallback, CStrAny sName, CRect<int> const *rc);
  
  COSWindow()          {}
  virtual ~COSWindow() {}

  virtual bool Init(CStartUp const &kStartUp, CCallback *pCallback, CStrAny sName, CRect<int> const *rc) = 0;
  virtual void Done() = 0;

  virtual CRect<int> const &GetRect() const = 0;
  virtual void SetRect(CRect<int> const &rc) = 0;

  virtual bool Process() = 0; // Returns false if the window needs to be closed
};

#endif