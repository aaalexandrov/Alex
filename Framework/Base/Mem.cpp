#include "StdAfx.h"
#include "Mem.h"

class CMemTest {
public:
  int m_iData;

  CMemTest(int i = 55): m_iData(i) {}
  ~CMemTest() { m_iData = -11; }
};

struct TTestAlloc {
  void *Alloc(size_t uiSize, const char *pFile, int iLine) { return new uint8_t[uiSize]; }
  void Free(void *p, const char *pFile, int iLine)         { delete[] (uint8_t *) p; }
};

template <>
struct TGetAllocator<CMemTest> {
  typedef TTestAlloc Type;
};

template <>
TTestAlloc &GetAllocatorInstance(CMemTest *)
{
  static TTestAlloc alloc;
  return alloc;
}

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
