#include "stdafx.h"
#include "Mem.h"
#include "Hash.h"

class CMemBase {};

class CMemTest: public CMemBase {
public:
  int m_iData;

  CMemTest(int i = 55): m_iData(i) {}
  ~CMemTest() { m_iData = -11; }

  operator size_t() const { return m_iData; }
};

struct TTestAlloc {
  inline void *Alloc(size_t uiSize, const char *pFile, int iLine) { return malloc(uiSize); }
  inline void Free(void *p, const char *pFile, int iLine)         { free(p); }
  inline size_t GetBlockSize(void *p)                             { return malloc_usable_size(p); }

  TTestAlloc() {}
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
    auto a = GetAllocatorInstance((CHash<CMemTest>::CHashList *) 0);

    int *n = NEW(int, ());
    DEL(n);

    Tst<int, 5> *nn = NEW(ID_TYPE(Tst<int, 5>), ());
    DEL(nn);

    Tst<int, 5> *na = NEWARR(ID_TYPE(Tst<int, 5>), 5);
    DELARR(5, na);

    CMemTest *pp = NEW(CMemTest, (44));
    DEL(pp);

    CHash<CMemTest> h;
    h.Add(CMemTest());
  }
} /*g_Test*/;
