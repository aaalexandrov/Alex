#ifndef __LIST_H
#define __LIST_H

#include "mem.h"

template <class T, class K = T, class P = Util::Equal<T> >
class CList {
public:
  typedef T Elem;

  struct TNode {
    TNode *pNext, *pPrev;
    T Data;

    TNode(T t): Data(t) {}
  };

  TNode *m_pHead, *m_pTail;
  int m_iCount;

  CList();
  ~CList();

  void DeleteAll();
	void Clear();

  void Push(T t);
  void PushTail(T t);

  T &Pop(T &t);
  T &PopTail(T &t);

  T &Head(T &t);
  T &Tail(T &t);

  T Pop();
	T PopTail();

	T Head() const;
	T Tail() const;

  void PushAfter(TNode *pAfter, T t);
  void PushBefore(TNode *pBefore, T t);

  void Remove(TNode *pNode);
  bool RemoveValue(T t);

	template <class K1>
	TNode *Find(K1 k) const;

  TNode *Find(K k) const { return Find<K>(k); }

  void PushNode(TNode *pNode);
  void PushNodeTail(TNode *pNode);

  TNode *PopNode();
  TNode *PopNodeTail();

  void PushNodeAfter(TNode *pAfter, TNode *pNode);
  void PushNodeBefore(TNode *pBefore, TNode *pNode);
};

// Implementation ------------------------------------------------------------------

// CList ---------------------------------------------------------------------------
template <class T, class K, class P>
CList<T, K, P>::CList()
{
  m_pHead = 0;
  m_pTail = 0;
  m_iCount = 0;
}

template <class T, class K, class P>
CList<T, K, P>::~CList()
{
  while (m_pHead) {
    TNode *p = m_pHead;
    m_pHead = m_pHead->pNext;
    DEL(p);
  }
}

template <class T, class K, class P>
void CList<T, K, P>::DeleteAll()
{
  while (m_pHead) {
    TNode *p = m_pHead;
    m_pHead = m_pHead->pNext;
    DEL(p->Data);
    DEL(p);
  }
  m_pTail = 0;
	m_iCount = 0;
}

template <class T, class K, class P>
void CList<T, K, P>::Clear()
{
  while (m_pHead) {
    TNode *p = m_pHead;
    m_pHead = m_pHead->pNext;
    DEL(p);
  }
  m_pTail = 0;
	m_iCount = 0;
}

template <class T, class K, class P>
void CList<T, K, P>::Push(T t)
{
  PushNode(NEW(TNode, (t)));
}

template <class T, class K, class P>
void CList<T, K, P>::PushTail(T t)
{
  PushNodeTail(NEW(TNode, (t)));
}

template <class T, class K, class P>
T &CList<T, K, P>::Pop(T &t)
{
  TNode *pNode = PopNode();
  if (pNode) {
    t = pNode->Data;
    DEL(pNode);
  }
  return t;
}

template <class T, class K, class P>
T &CList<T, K, P>::PopTail(T &t)
{
  TNode *pNode = PopNodeTail();
  if (pNode) {
    t = pNode->Data;
    DEL(pNode);
  }
  return t;
}

template <class T, class K, class P>
T &CList<T, K, P>::Head(T &t)
{
  if (m_pHead)
    t = m_pHead->Data;
  return t;
}

template <class T, class K, class P>
T &CList<T, K, P>::Tail(T &t)
{
  if (m_pTail)
    t = m_pTail->Data;
  return t;
}

template <class T, class K, class P>
T CList<T, K, P>::Pop()
{
	T t;
  return Pop(t);
}

template <class T, class K, class P>
T CList<T, K, P>::PopTail()
{
	T t;
  return PopTail(t);
}

template <class T, class K, class P>
T CList<T, K, P>::Head() const
{
	T t;
  if (m_pHead)
    t = m_pHead->Data;
  return t;
}

template <class T, class K, class P>
T CList<T, K, P>::Tail() const
{
	T t;
  if (m_pTail)
    t = m_pTail->Data;
  return t;
}

template <class T, class K, class P>
void CList<T, K, P>::PushAfter(TNode *pAfter, T t)
{
  PushNodeAfter(pAfter, NEW(TNode, (t)));
}

template <class T, class K, class P>
void CList<T, K, P>::PushBefore(TNode *pBefore, T t)
{
  PushNodeBefore(pBefore, NEW(TNode, (t)));
}

template <class T, class K, class P>
void CList<T, K, P>::Remove(TNode *pNode)
{
  if (!pNode)
    return;
  if (pNode->pNext)
    pNode->pNext->pPrev = pNode->pPrev;
  else
    m_pTail = pNode->pPrev;
  if (pNode->pPrev)
    pNode->pPrev->pNext = pNode->pNext;
  else
    m_pHead = pNode->pNext;
  DEL(pNode);
  m_iCount--;
}

template <class T, class K, class P>
bool CList<T, K, P>::RemoveValue(T t)
{
  TNode *pNode = Find(t);
  Remove(pNode);
  return !!pNode;
}

template <class T, class K, class P>
template <class K1>
typename CList<T, K, P>::TNode *CList<T, K, P>::Find(K1 k) const
{
  TNode *pNode;
  for (pNode = m_pHead; pNode && !P::Eq(k, pNode->Data); pNode = pNode->pNext);
  return pNode;
}

template <class T, class K, class P>
void CList<T, K, P>::PushNode(TNode *pNode)
{
  pNode->pNext = m_pHead;
  pNode->pPrev = 0;
  if (m_pHead)
    m_pHead->pPrev = pNode;
  else
    m_pTail = pNode;
  m_pHead = pNode;
  m_iCount++;
}

template <class T, class K, class P>
void CList<T, K, P>::PushNodeTail(TNode *pNode)
{
  pNode->pNext = 0;
  pNode->pPrev = m_pTail;
  if (m_pTail)
    m_pTail->pNext = pNode;
  else
    m_pHead = pNode;
  m_pTail = pNode;
  m_iCount++;
}

template <class T, class K, class P>
typename CList<T, K, P>::TNode *CList<T, K, P>::PopNode()
{
  TNode *pNode = m_pHead;
  if (m_pHead) {
    if (!m_pHead->pNext)
      m_pTail = 0;
    m_pHead = m_pHead->pNext;
    m_iCount--;
  }
  return pNode;
}

template <class T, class K, class P>
typename CList<T, K, P>::TNode *CList<T, K, P>::PopNodeTail()
{
  TNode *pNode = m_pTail;
  if (m_pTail) {
    if (!m_pTail->pPrev)
      m_pHead = 0;
    m_pTail = m_pTail->pPrev;
    m_iCount--;
  }
  return pNode;
}

template <class T, class K, class P>
void CList<T, K, P>::PushNodeAfter(TNode *pAfter, TNode *pNode)
{
  if (pAfter) {
    pNode->pPrev = pAfter;
    pNode->pNext = pAfter->pNext;
    if (!pAfter->pNext)
      m_pTail = pNode;
    pAfter->pNext = pNode;
    m_iCount++;
  } else
    PushNodeTail(pNode);
}

template <class T, class K, class P>
void CList<T, K, P>::PushNodeBefore(TNode *pBefore, TNode *pNode)
{
  if (pBefore) {
    pNode->pNext = pBefore;
    pNode->pPrev = pBefore->pPrev;
    if (!pBefore->pPrev)
      m_pHead = pNode;
    pBefore->pPrev = pNode;
    m_iCount++;
  } else
    PushNode(pNode);
}

#endif
