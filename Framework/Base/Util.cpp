#include "stdafx.h"
#include "Util.h"
#include "Base.h"
#include "Hash.h"

using namespace Util;

// ----------------------------------------------------------------------------

extern const int g_iHashSizes[];

class CRefCounted {
	DEFREFCOUNT
public:
	~CRefCounted() {
		int i = 0;
	}
};

static void UtilTest()
{
  int iInd;

  iInd = BinSearch<int *, int, LessV<int> >((int *) g_iHashSizes, ARRSIZE(g_iHashSizes), g_iHashSizes[ARRSIZE(g_iHashSizes) - 1]);
  iInd = BinSearch<int *, int, LessV<int> >((int *) g_iHashSizes, ARRSIZE(g_iHashSizes), 37);
  iInd = BinSearch<int *, int, LessV<int> >((int *) g_iHashSizes, ARRSIZE(g_iHashSizes), 11000);
  iInd = BinSearch<int *, int, LessV<int> >((int *) g_iHashSizes, ARRSIZE(g_iHashSizes), 5);
  iInd = BinSearch<int *, int, LessV<int> >((int *) g_iHashSizes, ARRSIZE(g_iHashSizes), 31);
  iInd = BinSearch<int *, int, LessV<int> >((int *) g_iHashSizes, ARRSIZE(g_iHashSizes), g_iHashSizes[ARRSIZE(g_iHashSizes) - 1] + 1);

	CSmartPtr<CRefCounted> ptr(NEW(CRefCounted, ()));
  CRefCounted *p;

	p = ptr;

  ASSERT(ptr);

	ptr = p;

  ptr = ptr;

	ptr->m_RefCount.m_dwCount++;
	(*ptr).Release(*CUR_ALLOC);

  int iBitTest[] = { 1, 5, 0, -5, 1024, 9857576, 468 };
  int i, j, iMSB, iLSB;
  for (i = 0; i < ARRSIZE(iBitTest); i++) {
    iMSB = MostSignificantBitSet(iBitTest[i]);
    for (j = sizeof(int) * 8 - 1; j >= 0; j--)
      if (iBitTest[i] & (1 << j))
        break;
    ASSERT(iMSB == j);
    iLSB = LeastSignificantBitSet(iBitTest[i]);
    for (j = 0; j < sizeof(int) * 8; j++)
      if (iBitTest[i] & (1 << j))
        break;
    if (j >= sizeof(int) * 8)
      j = -1;
    ASSERT(iLSB == j);
  }
}

/*
static struct TUtilTest {
  TUtilTest() { UtilTest(); }
} g_UtilTest;
*/
