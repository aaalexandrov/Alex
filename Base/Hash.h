#ifndef __HASH_H
#define __HASH_H

#include "List.h"
#include "Array.h"
#include "Util.h"

extern const int g_iHashSizes[68];

template <class T, class K = T, class H = Util::HashSize_T, class P = Util::Equal<T> >
class CHash {
public:
  const static int RESIZE_RATIO = 16;

  typedef T Elem;

  class TIter {
  public:
    const CHash *m_pHash;
    int m_iList;
    typename CList<T, K, P>::TNode *m_pNode;

    TIter(const CHash *pHash = 0, int iDirection = 1);
    TIter(const TIter &it);

    void Init(const CHash *pHash, int iDirection);

    TIter &operator =(CHash &hash);
    TIter &operator ++();
    TIter &operator --();
    operator bool () const;
    T &operator *();
    T &operator ->();
    T const &operator ->() const;
  };

  typedef CArray<CList<T, K, P> *, 0> CHashArray;
  CHashArray m_arrLists;
  int m_iCount;

  CHash(int iSize = 0);
  ~CHash();

  void DeleteAll(int iSize = 0);
  void Clear(int iSize = 0);

  void Resize(int iSize);
  void CheckForResize();

  void Add(T t);
  void AddUnique(T t);
  void Remove(TIter it);
  bool RemoveValue(T t);
	template <class K1>
	TIter Find(K1 k) const;
  TIter Find(K k) const { return Find<K>(k); }
};

template <class K, class V, class H = Util::HashSize_T, class P = Util::Equal<K> >
class CHashKV: public CHash<Util::TKeyValue<K, V, H, P>, K, Util::TKeyValue<K, V, H, P>, Util::TKeyValue<K, V, H, P> > {
public:
  CHashKV(int iSize = 0): CHash(iSize) {}
};
// Implementation ------------------------------------------------------------------

// CHash ---------------------------------------------------------------------------

template <class T, class K, class H, class P>
CHash<T, K, H, P>::TIter::TIter(const CHash *pHash, int iDirection)
{
  Init(pHash, iDirection);
}

template <class T, class K, class H, class P>
CHash<T, K, H, P>::TIter::TIter(const TIter &it)
{
  m_pHash = it.m_pHash;
  m_iList = it.m_iList;
  m_pNode = it.m_pNode;
}

template <class T, class K, class H, class P>
void CHash<T, K, H, P>::TIter::Init(const CHash *pHash, int iDirection)
{
  m_pHash = pHash;
  if (pHash) {
    if (iDirection > 0) {
      m_iList = 0;
      while (m_iList < m_pHash->m_arrLists.m_iCount && (!m_pHash->m_arrLists[m_iList] || !m_pHash->m_arrLists[m_iList]->m_pHead))
        m_iList++;
      if (m_iList < m_pHash->m_arrLists.m_iCount)
        m_pNode = m_pHash->m_arrLists[m_iList]->m_pHead;
      else
        m_pNode = 0;
    } else
      if (iDirection < 0) {
        m_iList = m_pHash->m_arrLists.m_iCount - 1;
        while (m_iList >= 0 && (!m_pHash->m_arrLists[m_iList] || !m_pHash->m_arrLists[m_iList]->m_pTail))
          m_iList--;
        if (m_iList >= 0)
          m_pNode = m_pHash->m_arrLists[m_iList]->m_pTail;
        else
          m_pNode = 0;
      } else {
        m_iList = 0;
        m_pNode = 0;
      }
  } else {
    m_iList = 0;
    m_pNode = 0;
  }
}

template <class T, class K, class H, class P>
typename CHash<T, K, H, P>::TIter &CHash<T, K, H, P>::TIter::operator =(CHash &hash)
{
  Init(&hash, 1);
  return *this;
}

template <class T, class K, class H, class P>
typename CHash<T, K, H, P>::TIter &CHash<T, K, H, P>::TIter::operator ++()
{
  ASSERT(m_pNode);
  m_pNode = m_pNode->pNext;
  if (!m_pNode) {
    do {
      m_iList++;
    } while (m_iList < m_pHash->m_arrLists.m_iCount && (!m_pHash->m_arrLists[m_iList] || !m_pHash->m_arrLists[m_iList]->m_pHead));
    if (m_iList < m_pHash->m_arrLists.m_iCount)
      m_pNode = m_pHash->m_arrLists[m_iList]->m_pHead;
  }
  return *this;
}

template <class T, class K, class H, class P>
typename CHash<T, K, H, P>::TIter &CHash<T, K, H, P>::TIter::operator --()
{
  ASSERT(m_pNode);
  m_pNode = m_pNode->pPrev;
  if (!m_pNode) {
    do {
      m_iList--;
    } while (m_iList >= 0 && (!m_pHash->m_arrLists[m_iList] || !m_pHash->m_arrLists[m_iList]->m_pTail));
    if (m_iList >= 0)
      m_pNode = m_pHash->m_arrLists[m_iList]->m_pTail;
  }
  return *this;
}

template <class T, class K, class H, class P>
CHash<T, K, H, P>::TIter::operator bool () const
{
  return !!m_pNode;
}

template <class T, class K, class H, class P>
T &CHash<T, K, H, P>::TIter::operator *()
{
  ASSERT(m_pNode);
  return m_pNode->Data;
}

template <class T, class K, class H, class P>
T &CHash<T, K, H, P>::TIter::operator ->()
{
  ASSERT(m_pNode);
  return m_pNode->Data;
}

template <class T, class K, class H, class P>
T const &CHash<T, K, H, P>::TIter::operator ->() const
{
  ASSERT(m_pNode);
  return m_pNode->Data;
}

template <class T, class K, class H, class P>
CHash<T, K, H, P>::CHash(int iSize): m_arrLists(iSize ? iSize : g_iHashSizes[0])
{
  m_iCount = 0;
  m_arrLists.SetCount(m_arrLists.m_iMaxCount);
  for (int i = 0; i < m_arrLists.m_iCount; i++)
    m_arrLists[i] = 0;
}

template <class T, class K, class H, class P>
CHash<T, K, H, P>::~CHash()
{
  m_arrLists.DeleteAll();
}

template <class T, class K, class H, class P>
void CHash<T, K, H, P>::DeleteAll(int iSize)
{
  int i;
  if (!iSize)
    iSize = g_iHashSizes[0];
  for (i = 0; i < m_arrLists.m_iCount; i++) {
    CList<T, K, P> *&pLst = m_arrLists[i];
    if (pLst)
      pLst->DeleteAll();
  }
  m_arrLists.DeleteAll(iSize);
  m_iCount = 0;
  m_arrLists.SetCount(iSize);
  for (i = 0; i < m_arrLists.m_iCount; i++)
    m_arrLists[i] = 0;
}

template <class T, class K, class H, class P>
void CHash<T, K, H, P>::Clear(int iSize)
{
  int i;
  if (!iSize)
    iSize = g_iHashSizes[0];
  m_arrLists.DeleteAll(iSize);
  m_iCount = 0;
  m_arrLists.SetCount(iSize);
  for (i = 0; i < m_arrLists.m_iCount; i++)
    m_arrLists[i] = 0;
}

template <class T, class K, class H, class P>
void CHash<T, K, H, P>::Resize(int iSize)
{
  ASSERT(iSize != m_arrLists.m_iCount);
  CHash<T, K, H, P> hash(iSize);
  Util::Swap(hash.m_arrLists.m_pArray, m_arrLists.m_pArray);
  Util::Swap(hash.m_arrLists.m_iCount, m_arrLists.m_iCount);
  Util::Swap(hash.m_arrLists.m_iMaxCount, m_arrLists.m_iMaxCount);
  Util::Swap(hash.m_iCount, m_iCount);
  TIter it(&hash), it1;
  while (it) {
    it1 = it;
    ++it;
    Add(*it1);
    hash.Remove(it1);
  }
}

template <class T, class K, class H, class P>
void CHash<T, K, H, P>::CheckForResize()
{
  int i, iSize = m_iCount / RESIZE_RATIO;
  if (iSize > m_arrLists.m_iCount) {
    iSize += iSize / 6;
    for (i = 0; i < (int) ARRSIZE(g_iHashSizes) && g_iHashSizes[i] < iSize; i++);
    if (i < (int) ARRSIZE(g_iHashSizes))
      iSize = g_iHashSizes[i];
    else
      iSize = m_arrLists.m_iCount + m_arrLists.m_iCount / 4;
    Resize(iSize);
  }
}

template <class T, class K, class H, class P>
void CHash<T, K, H, P>::Add(T t)
{
  size_t uiHash = H::Hash(t);
  uiHash %= m_arrLists.m_iCount;
  CList<T, K, P> *&pLst = m_arrLists[(int) uiHash];
  if (!pLst)
    pLst = new CList<T, K, P>();
/*
  typename CList<T, K, P>::TNode *pNode;
  pNode = pLst->m_pHead;
  while (pNode && pNode->Data < t)
    pNode = pNode->pNext;
  pLst->PushBefore(pNode, t);
*/
  pLst->PushTail(t);
  m_iCount++;
  CheckForResize();
}

template <class T, class K, class H, class P>
void CHash<T, K, H, P>::AddUnique(T t)
{
  size_t uiHash = H::Hash(t);
  uiHash %= m_arrLists.m_iCount;
  CList<T, K, P> *&pLst = m_arrLists[(int) uiHash];
  if (!pLst)
    pLst = new CList<T, K, P>();
  CList<T, K, P>::TNode *pNode = pLst->Find(t);
  if (pNode) 
    pNode->Data = t;
  else {
    pLst->PushTail(t);
    m_iCount++;
    CheckForResize();
  }
}

template <class T, class K, class H, class P>
void CHash<T, K, H, P>::Remove(TIter it)
{
  ASSERT(it.m_pHash == this);
  if (!it)
    return;
  CList<T, K, P> *pLst = m_arrLists[it.m_iList];
  pLst->Remove(it.m_pNode);
  m_iCount--;
}

template <class T, class K, class H, class P>
bool CHash<T, K, H, P>::RemoveValue(T t)
{
  TIter it = Find(t);
  Remove(it);
  return !!it;
}

template <class T, class K, class H, class P>
template <class K1>
typename CHash<T, K, H, P>::TIter CHash<T, K, H, P>::Find(K1 k) const
{
  size_t uiHash = H::Hash(k);
  TIter it;
  it.m_pHash = this;
  uiHash %= m_arrLists.m_iCount;
  it.m_iList = (int) uiHash;
  CList<T, K, P> *pLst = m_arrLists[(int) uiHash];
  if (!pLst) {
    it.m_pNode = 0;
    return it;
  }
/*  for (it.m_pNode = pLst->m_pHead; it.m_pNode && it.m_pNode->Data < k; it.m_pNode = it.m_pNode->pNext);
  if (it.m_pNode && k < it.m_pNode->Data)
    it.m_pNode = 0;
*/
  it.m_pNode = pLst->Find<K1>(k);
  return it;
}

#endif
