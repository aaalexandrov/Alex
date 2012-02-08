#include "stdafx.h"
#include "Rect.h"

struct TRectTest {
  TRectTest() {
    CRect<> rc, rc1, rc2, rc3;
    rc.SetEmpty();
    bool b = rc.IsEmpty();
    rc1.Set(0, 0, 1, 1);
    rc2 = CRect<>(CVector<2>::Get(0.5f, 0.5f), CVector<2>::Get(1.5f, 1.5f));

    rc = rc1.GetIntersection(rc2);
    rc3 = rc2;
    rc1.MoveInside(rc3);
    b = rc == rc3;
  }
} /*g_RectTest*/;