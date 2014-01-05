#include "stdafx.h"
#include "AVLTree.h"

static void AVLTreeTest()
{
  CAVLTree<int *> kTree;
  CAVLTree<int *>::TIter it;
  int i, iInd;
  int *pValues[10000];
  bool bFirst = true;
  for (i = 0; i < 10000; i++) {
    iInd = rand();
    pValues[i] = (int *) (intptr_t) iInd;
    if (bFirst) {
      kTree.Add((int *) (intptr_t) iInd);
      bFirst = false;
    } else
      kTree.AddUnique((int *) (intptr_t) iInd);
    ASSERT(kTree.CheckIntegrity());
    it = kTree.Find((int *) (intptr_t) iInd);
    ASSERT(it && *it == (int *) (intptr_t) iInd);
  }
  Util::QSort<int *[10000], int *, Util::LessV<int *> >(pValues, 10000);
  for (i = 0, it = kTree; i < 10000; ++it) {
    int *pVal = pValues[i];
    ASSERT(*it == pVal);
    while (i < 10000 && pValues[i] == pVal)
      i++;
  }
  bool bRemoveByVal = false;
  while (kTree.m_iCount) {
    iInd = rand() % kTree.m_iCount;
    for (i = 0, it = kTree; i < iInd; i++, ++it);
    ASSERT(it);
    if (bRemoveByVal)
      kTree.RemoveValue(*it);
    else
      kTree.Remove(it);
    bRemoveByVal = !bRemoveByVal;
    ASSERT(kTree.CheckIntegrity());
  }
}

/*
struct TAVLTreeTest {
  TAVLTreeTest() { AVLTreeTest(); }
} g_AVLTreeTest;
*/
