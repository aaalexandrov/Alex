#ifndef __LIST_H
#define __LIST_H


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

  void PushAfter(TNode *pNode, T t);
  void PushBefore(TNode *pNode, T t);

  void Remove(TNode *pNode);
  bool RemoveValue(T t);

	template <class K1>
	TNode *Find(K1 k) const;

  TNode *Find(K k) const { return Find<K>(k); }
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
    delete p;
  }
}

template <class T, class K, class P>
void CList<T, K, P>::DeleteAll()
{
  while (m_pHead) {
    TNode *p = m_pHead;
    m_pHead = m_pHead->pNext;
    delete p->Data;
    delete p;
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
    delete p;
  }
  m_pTail = 0;
	m_iCount = 0;
}

template <class T, class K, class P>
void CList<T, K, P>::Push(T t)
{
  TNode *p = new TNode(t);
  p->pNext = m_pHead;
  p->pPrev = 0;
  if (m_pHead)
    m_pHead->pPrev = p;
  else
    m_pTail = p;
  m_pHead = p;
  m_iCount++;
}

template <class T, class K, class P>
void CList<T, K, P>::PushTail(T t)
{
  TNode *p = new TNode(t);
  p->pNext = 0;
  p->pPrev = m_pTail;
  if (m_pTail)
    m_pTail->pNext = p;
  else
    m_pHead = p;
  m_pTail = p;
  m_iCount++;
}

template <class T, class K, class P>
T &CList<T, K, P>::Pop(T &t)
{
  if (m_pHead) {
    t = m_pHead->Data;
    if (!m_pHead->pNext)
      m_pTail = 0;
    TNode *pNode = m_pHead;
    m_pHead = m_pHead->pNext;
    delete pNode;
    m_iCount--;
  }
  return t;
}

template <class T, class K, class P>
T &CList<T, K, P>::PopTail(T &t)
{
  if (m_pTail) {
    t = m_pTail->Data;
    if (!m_pTail->pPrev)
      m_pHead = 0;
    TNode *pNode = m_pTail;
    m_pTail = m_pTail->pPrev;
    delete pNode;
    m_iCount--;
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
  if (m_pHead) {
    t = m_pHead->Data;
    if (!m_pHead->pNext)
      m_pTail = 0;
    TNode *pNode = m_pHead;
    m_pHead = m_pHead->pNext;
    delete pNode;
    m_iCount--;
  }
  return t;
}

template <class T, class K, class P>
T CList<T, K, P>::PopTail()
{
	T t;
  if (m_pTail) {
    t = m_pTail->Data;
    if (!m_pTail->pPrev)
      m_pHead = 0;
    TNode *pNode = m_pTail;
    m_pTail = m_pTail->pPrev;
    delete pNode;
    m_iCount--;
  }
  return t;
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
void CList<T, K, P>::PushAfter(TNode *pNode, T t)
{
  TNode *p = new TNode(t);
  if (pNode) {
    p->pPrev = pNode;
    p->pNext = pNode->pNext;
    if (!pNode->pNext)
      m_pTail = p;
    pNode->pNext = p;
    m_iCount++;
  } else
    PushTail(t);
}

template <class T, class K, class P>
void CList<T, K, P>::PushBefore(TNode *pNode, T t)
{
  TNode *p = new TNode(t);
  if (pNode) {
    p->pNext = pNode;
    p->pPrev = pNode->pPrev;
    if (!pNode->pPrev)
      m_pHead = p;
    pNode->pPrev = p;
    m_iCount++;
  } else
    Push(t);
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
  delete pNode;
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

#endif
