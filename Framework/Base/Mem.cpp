#include "StdAfx.h"
#include "Mem.h"

class CMemTest {
public:
  int m_iData;

  CMemTest(int i = 55): m_iData(i) {}
  ~CMemTest() { m_iData = -11; }
};

template <class T, int I>
struct Tst {
  T arr[I];
};

struct TMemTest {
  TMemTest()
  {
    int *n = NEW(int, ());
    DEL(n);

    Tst<int, 5> *nn = NEW(ID_TYPE(Tst<int, 5>), ());
    DEL(nn);

    Tst<int, 5> *na = NEWARR(ID_TYPE(Tst<int, 5>), 5);
    DELARR(5, na);

    CMemTest *pp = NEW(CMemTest, (44));
    DEL(pp);
  }
} /*g_Test*/;
