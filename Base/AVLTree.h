#ifndef __AVLTREE_H
#define __AVLTREE_H

#include "Util.h"

template <class T, class K = T, class P = Util::Less<T> >
class CAVLTree {
public:
  typedef T Elem;

  struct TNode {
    T Data;
    TNode *pChild[2], *pParent;
    uint8_t btLevel;

    TNode(T t): Data(t), btLevel(0) { pParent = pChild[0] = pChild[1] = 0; }

    void SetChild(int iChild, TNode *pN);
    void SetLevel()                    { SetLevel(this); }
    inline int GetChildDir(TNode *pCh) { return pChild[1] == pCh; }

    static inline void SetLevel(TNode *pNode);
  };

  class TIter {
  public:
    TNode *m_pNode;

    TIter(CAVLTree *pTree = 0, int iDirection = 0) { Assign(pTree, iDirection); }
    TIter(const TIter &it): m_pNode(it.m_pNode)    {}

    TIter &operator =(CAVLTree &tree) { return Assign(&tree, 0); }
    TIter &operator ++()              { return Increment(0); }
    TIter &operator --()              { return Increment(1); }
    operator bool () const            { return !!m_pNode; }
    T &operator *()                   { ASSERT(m_pNode); return m_pNode->Data; }
    T &operator ->()                  { ASSERT(m_pNode); return m_pNode->Data; }

    TIter &Increment(int iDir);
    TIter &Assign(CAVLTree *pTree, int iDir);
  };

public:
  TNode *m_pRoot;
  int m_iCount;

  CAVLTree()  { m_pRoot = 0; m_iCount = 0; }
  ~CAVLTree() { Clear(); }

  void DeleteAll() { DeleteAll(m_pRoot); m_pRoot = 0; m_iCount = 0; }
  void Clear()     { Clear(m_pRoot); m_pRoot = 0; m_iCount = 0; }

  void Add(T t);
  void AddUnique(T t);
  void Remove(TIter it);
  bool RemoveValue(K k);
  template <class K1>
  TIter Find(K1 k) const;
  TIter Find(K k) const { return Find<K>(k); }

  bool Balance(TNode *pNode);
  bool CheckIntegrity();

  static inline int GetNodeLevel(TNode *pNode) { if (!pNode) return -1; return pNode->btLevel; }
  static inline void DeleteAll(TNode *pNode);
	static inline void Clear(TNode *pNode);
  static bool CheckIntegrity(TNode *pNode, int &iCount, TNode *pParent = 0);
};

template <class K, class V, class L = Less<K> >
class CAVLTreeKV: public CAVLTree<Util::TKeyValue<K, V, Util::HashSize_T, Util::Equal<K>, L>, K, Util::TKeyValue<K, V, Util::HashSize_T, Util::Equal<K>, L> > {
public:
  CAVLTreeKV(): CAVLTree<Util::TKeyValue<K, V, Util::HashSize_T, Util::Equal<K>, L>, K, Util::TKeyValue<K, V, Util::HashSize_T, Util::Equal<K>, L> >() {}
};

// Implementation ------------------------------------------------------------------

// CAVLTree::TNode -----------------------------------------------------------------

template <class T, class K, class P>
void CAVLTree<T, K, P>::TNode::SetChild(int iChild, TNode *pN)
{
  pChild[iChild] = pN;
  if (pChild[iChild])
    pChild[iChild]->pParent = this;
  SetLevel();
}

template <class T, class K, class P>
void CAVLTree<T, K, P>::TNode::SetLevel(TNode *pNode)
{
  uint8_t btL;
  do {
    btL = Util::Max(GetNodeLevel(pNode->pChild[0]), GetNodeLevel(pNode->pChild[1])) + 1;
    if (btL != pNode->btLevel) {
      pNode->btLevel = btL;
      pNode = pNode->pParent;
    } else
      pNode = 0;
  } while (pNode);
}

// CAVLTree::TIter -----------------------------------------------------------------

template <class T, class K, class P>
typename CAVLTree<T, K, P>::TIter &CAVLTree<T, K, P>::TIter::Increment(int iDir)
{
  ASSERT(m_pNode);
  if (m_pNode->pChild[!iDir]) {
    m_pNode = m_pNode->pChild[!iDir];
    while (m_pNode->pChild[iDir])
      m_pNode = m_pNode->pChild[iDir];
  } else {
    while (m_pNode && (!m_pNode->pParent || m_pNode == m_pNode->pParent->pChild[!iDir]))
      m_pNode = m_pNode->pParent;
    if (m_pNode)
      m_pNode = m_pNode->pParent;
  }
  return *this;
}

template <class T, class K, class P>
typename CAVLTree<T, K, P>::TIter &CAVLTree<T, K, P>::TIter::Assign(CAVLTree *pTree, int iDir)
{
  m_pNode = pTree ? pTree->m_pRoot : 0;
  if (m_pNode)
    while (m_pNode->pChild[iDir])
      m_pNode = m_pNode->pChild[iDir];
  return *this;
}

// CAVLTree ------------------------------------------------------------------------

template <class T, class K, class P>
void CAVLTree<T, K, P>::Add(T t)
{
  if (!m_pRoot) {
    m_pRoot = new TNode(t);
    m_pRoot->pParent = 0;
    m_iCount = 1;
    return;
  }
  TNode *pNode;
  pNode = m_pRoot;
  while (1) {
    int iChild;
    iChild = !(P::Lt(t, pNode->Data));
    if (pNode->pChild[iChild])
      pNode = pNode->pChild[iChild];
    else {
      uint8_t btLevel = pNode->btLevel;
      pNode->SetChild(iChild, new TNode(t));
      if (btLevel != pNode->btLevel)
        while (pNode->pParent && !Balance(pNode->pParent))
          pNode = pNode->pParent;
      m_iCount++;
      break;
    }
  }
}

template <class T, class K, class P>
void CAVLTree<T, K, P>::AddUnique(T t)
{
  if (!m_pRoot) {
    m_pRoot = new TNode(t);
    m_pRoot->pParent = 0;
    m_iCount = 1;
    return;
  }
  TNode *pNode;
  pNode = m_pRoot;
  while (1) {
    int iChild;
    if (P::Lt(t, pNode->Data))
      iChild = 0;
    else
      if (P::Lt(pNode->Data, t))
        iChild = 1;
      else {
        pNode->Data = t;
        break;
      }
    if (pNode->pChild[iChild])
      pNode = pNode->pChild[iChild];
    else {
      uint8_t btLevel = pNode->btLevel;
      pNode->SetChild(iChild, new TNode(t));
      if (btLevel != pNode->btLevel)
        while (pNode->pParent && !Balance(pNode->pParent))
          pNode = pNode->pParent;
      m_iCount++;
      break;
    }
  }
}

template <class T, class K, class P>
void CAVLTree<T, K, P>::Remove(TIter it)
{
  ASSERT(it);
  TNode *pChild[2], *pNode, *pParent, *pPar;
  int iChild;
  pChild[0] = it.m_pNode->pChild[0];
  pChild[1] = it.m_pNode->pChild[1];
  pParent = it.m_pNode->pParent;
  if (pChild[0])
    iChild = 1;
  else
    if (pChild[1])
      iChild = 0;
    else
      iChild = -1;
  if (iChild >= 0) {
    pNode = pChild[!iChild];
    while (pNode->pChild[iChild])
      pNode = pNode->pChild[iChild];
    if (pNode != pChild[!iChild]) {
      pPar = pNode->pParent;
      pPar->SetChild(pPar->GetChildDir(pNode), pNode->pChild[!iChild]);
      pNode->pParent = 0;
      pNode->SetChild(0, pChild[0]);
      pNode->SetChild(1, pChild[1]);
    } else {
      pPar = pNode;
      pNode->pParent = 0;
      pNode->SetChild(iChild, pChild[iChild]);
    }
  } else {
    pPar = pParent;
    pNode = 0;
  }
  if (pParent)
    pParent->SetChild(pParent->GetChildDir(it.m_pNode), pNode);
  else
    m_pRoot = pNode;
  delete it.m_pNode;
  while (pPar) {
    Balance(pPar);
    pPar = pPar->pParent;
  }
  m_iCount--;
}

template <class T, class K, class P>
bool CAVLTree<T, K, P>::RemoveValue(K k)
{
  TIter it = Find(k);
  if (it) {
    Remove(it);
    return true;
  }
  return false;
}

template <class T, class K, class P>
template <class K1>
typename CAVLTree<T, K, P>::TIter CAVLTree<T, K, P>::Find(K1 k) const
{
  TIter it;
  it.m_pNode = m_pRoot;
  while (it) {
    if (P::Lt(k, it.m_pNode->Data))
      it.m_pNode = it.m_pNode->pChild[0];
    else
      if (P::Lt(it.m_pNode->Data, k))
        it.m_pNode = it.m_pNode->pChild[1];
      else
        break;
  }
  return it;
}

template <class T, class K, class P>
bool CAVLTree<T, K, P>::CheckIntegrity()
{
  int iCount = 0;
  bool bRes = CheckIntegrity(m_pRoot, iCount);
  return bRes && iCount == m_iCount;
}

template <class T, class K, class P>
void CAVLTree<T, K, P>::DeleteAll(TNode *pNode)
{
  if (!pNode)
    return;
  DeleteAll(pNode->pChild[0]);
  DeleteAll(pNode->pChild[1]);
  delete pNode->Data;
  delete pNode;
}

template <class T, class K, class P>
void CAVLTree<T, K, P>::Clear(TNode *pNode)
{
  if (!pNode)
    return;
  Clear(pNode->pChild[0]);
  Clear(pNode->pChild[1]);
  delete pNode;
}

template <class T, class K, class P>
bool CAVLTree<T, K, P>::Balance(TNode *pNode)
{
  int iDelta, iChild, iParentChild;
  TNode *pTemp1, *pTemp2, *pTemp3, *pTemp4, *pParent;
  iDelta = GetNodeLevel(pNode->pChild[1]) - GetNodeLevel(pNode->pChild[0]);
  if (iDelta < -1)
    iChild = 0;
  else
    if (iDelta > 1)
      iChild = 1;
    else
      return false;
  iDelta = GetNodeLevel(pNode->pChild[iChild]->pChild[!iChild]) - GetNodeLevel(pNode->pChild[iChild]->pChild[iChild]);
  ASSERT(abs(iDelta) < 2);
  pParent = pNode->pParent;
  if (pParent) {
    iParentChild = pParent->GetChildDir(pNode);
    pNode->pParent = 0;
  }
  if (iDelta <= 0) {
    pTemp1 = pNode->pChild[iChild];
    pTemp2 = pTemp1->pChild[!iChild];
    pNode->SetChild(iChild, pTemp2);
    pTemp1->SetChild(!iChild, pNode);
  } else {
    pTemp2 = pNode->pChild[iChild];
    pTemp1 = pTemp2->pChild[!iChild];
    pTemp3 = pTemp1->pChild[iChild];
    pTemp4 = pTemp1->pChild[!iChild];
    pNode->SetChild(iChild, pTemp4);
    pTemp2->SetChild(!iChild, pTemp3);
    pTemp1->SetChild(iChild, pTemp2);
    pTemp1->SetChild(!iChild, pNode);
  }
  if (pParent)
    pParent->SetChild(iParentChild, pTemp1);
  else {
    m_pRoot = pTemp1;
    pTemp1->pParent = 0;
  }
  return true;
}

template <class T, class K, class P>
bool CAVLTree<T, K, P>::CheckIntegrity(TNode *pNode, int &iCount, TNode *pParent)
{
  bool bRes = true;
  if (!pNode)
    return bRes;
  iCount++;
  if (pNode->pParent != pParent)
    bRes = false;
  if (pParent)
    if (pParent->GetChildDir(pNode)) {
      if (P::Lt(pNode->Data, pParent->Data))
        bRes = false;
    } else
      if (P::Lt(pParent->Data, pNode->Data))
        bRes = false;
  int iLevel0, iLevel1;
  iLevel0 = GetNodeLevel(pNode->pChild[0]);
  iLevel1 = GetNodeLevel(pNode->pChild[1]);
  if (pNode->btLevel != Util::Max(iLevel0, iLevel1) + 1)
    bRes = false;
  if (abs(iLevel1 - iLevel0) > 1)
    bRes = false;
  bRes &= CheckIntegrity(pNode->pChild[0], iCount, pNode);
  bRes &= CheckIntegrity(pNode->pChild[1], iCount, pNode);
  return bRes;
}

#endif
