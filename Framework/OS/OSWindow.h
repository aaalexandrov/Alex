#ifndef __OSWINDOW_H
#define __OSWINDOW_H

#include "RTTI.h"
#include "Rect.h"

class COSWindow: public CObject {
  DEFRTTI(COSWindow, CObject, false)
public:
  COSWindow()          {}
  virtual ~COSWindow() {}

  virtual bool Init(CStrAny sName, CRect<int> const *rc) = 0;
  virtual void Done() = 0;

  virtual CRect<int> const &GetRect() const = 0;
  virtual void SetRect(CRect<int> const &rc) = 0;

  virtual bool Process() = 0; // Returns false if the window needs to be closed
  
  virtual void OnMove(CRect<int> const &rcNew)   {}
  virtual bool OnDraw(CRect<int> const &rcDirty) { return false; } // Returns true if the rect was actually drawn by the method
};

#endif