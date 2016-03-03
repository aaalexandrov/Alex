#ifndef __ARRAY_H
#define __ARRAY_H

#include "Debug.h"
#include "Util.h"
#include "Mem.h"

template <int G, int M, bool N>
struct TArrayGrow {
  static int GetGrowInc(int iMaxCount) { return G; }
};

template <int G, int M>
struct TArrayGrow<G, M, true> {
  static int GetGrowInc(int iMaxCount) { return Util::Max(M, iMaxCount / -G); }
};

template <class T, int G = -4>
class CArray {
public:
  static const int GROW_MIN = 8;
  static const int GROW_INC = G;
  // GROW_INC > 0 : linear increment;
  // GROW_INC == 0 : autoincrement off, no additional space for future count increases reserved;
  // GROW_INC < 0 : proportional increment, equal to Max(GROW_MIN, m_iMaxCount / -GROW_INC);

  typedef T Elem;

  T *m_pArray;
  int m_iCount, m_iMaxCount;

  CArray(int iMaxCount = 16);
  ~CArray();

  void DeleteAll(int iMaxCount = 16);
  void Clear(int iMaxCount = 16);
	bool IsEmpty() const { return !m_iCount; }

  T &operator [](int i);
  T const &operator [](int i) const;

  T &At(int i);
  T const &At(int i) const;

	T *PtrAt(int i);
	T const *PtrAt(int i) const;

  void SetCount(int iCount);
  void SetMaxCount(int iMaxCount);

	T &Last()               { return At(m_iCount - 1); }
  T const &Last() const   { return At(m_iCount - 1); }
  T &PreLast()               { return At(m_iCount - 2); }
  T const &PreLast() const   { return At(m_iCount - 2); }
  void Append(T const &t);

  int GetGrowInc() { return TArrayGrow<GROW_INC, GROW_MIN, GROW_INC < 0>::GetGrowInc(m_iMaxCount); /*if (GROW_INC >= 0) return GROW_INC; else return Util::Max(GROW_MIN, m_iMaxCount / -GROW_INC);*/ }
};

template <class T, class K = T, class P = Util::Less<T>, int G = -4>
class CSortedArray: public CArray<T, G> {
public:
  using CArray<T, G>::m_iCount;
  using CArray<T, G>::m_pArray;

  CSortedArray(int iMaxCount = 16);

  int Add(T t);
  T Remove(int i);
  int RemoveValue(T t);
  template <class K1>
  int Find(K1 k) const;
  int Find(K k) const { return Find<K>(k); }
};

template <class T, class K = T, class P = Util::Less<T>, int G = -4>
class CHeap: public CArray<T, G> {
public:
  using CArray<T, G>::m_iCount;
  using CArray<T, G>::m_pArray;
  using CArray<T, G>::Append;
  using CArray<T, G>::SetCount;

  CHeap(int iMaxCount = 16);

  void InitHeap();

  int Add(T t);
  T Remove(int i);
  int RemoveValue(T t);
  int Resort(int i);
  template <class K1>
  int Find(K1 k, int iRoot = 0) const;
  int Find(K k) const { return Find<K>(k); }

  int SiftUp(int i);
  int SiftDown(int i);

  static int UpIndex(int i)   { return (i + 1) / 2 - 1; }
  static int DownIndex(int i) { return (i + 1) * 2 - 1; }
};

// Implementation ------------------------------------------------------------------

// CArray --------------------------------------------------------------------------

template <class T, int G>
CArray<T, G>::CArray(int iMaxCount)
{
  m_iCount = 0;
  m_iMaxCount = iMaxCount;
  m_pArray = (T *) NEWARR(uint8_t, m_iMaxCount * sizeof(T));
}

template <class T, int G>
CArray<T, G>::~CArray()
{
	TConDestructor<T>::Destroy(m_pArray, m_iCount);
	DELARR(m_iMaxCount * sizeof(T), (uint8_t *) m_pArray);
}

template <class T, int G>
void CArray<T, G>::DeleteAll(int iMaxCount)
{
  for (int i = 0; i < m_iCount; i++) {
    DELARR(1, m_pArray[i]);
		TConDestructor<T>::Destroy(m_pArray + i);
	}
  m_iCount = 0;
  if (iMaxCount > 0)
    SetMaxCount(iMaxCount);
}

template <class T, int G>
void CArray<T, G>::Clear(int iMaxCount)
{
	TConDestructor<T>::Destroy(m_pArray, m_iCount);
  m_iCount = 0;
  if (iMaxCount > 0)
    SetMaxCount(iMaxCount);
}

template <class T, int G>
T &CArray<T, G>::operator [](int i)
{
  ASSERT(i >= 0 && i < m_iCount);
  return m_pArray[i];
}

template <class T, int G>
T const &CArray<T, G>::operator [](int i) const
{
  ASSERT(i >= 0 && i < m_iCount);
  return m_pArray[i];
}

template <class T, int G>
T &CArray<T, G>::At(int i)
{
  ASSERT(i >= 0 && i < m_iCount);
  return m_pArray[i];
}

template <class T, int G>
T const &CArray<T, G>::At(int i) const
{
  ASSERT(i >= 0 && i < m_iCount);
  return m_pArray[i];
}

template <class T, int G>
T *CArray<T, G>::PtrAt(int i)
{
	if (i < 0 || i >= m_iCount)
		return 0;
	return m_pArray + i;
}

template <class T, int G>
T const *CArray<T, G>::PtrAt(int i) const
{
	if (i < 0 || i >= m_iCount)
		return 0;
	return m_pArray + i;
}

template <class T, int G>
void CArray<T, G>::SetCount(int iCount)
{
  if (iCount > m_iMaxCount) {
    int iInc;
    iInc = GetGrowInc();
    if (iCount <= m_iMaxCount + iInc)
      SetMaxCount(m_iMaxCount + iInc);
    else
      SetMaxCount(iCount + iInc);
  }
	if (iCount > m_iCount)
		TConDestructor<T>::Construct(m_pArray + m_iCount, iCount - m_iCount);
	else
		if (iCount < m_iCount)
			TConDestructor<T>::Destroy(m_pArray + iCount, m_iCount - iCount);
  ASSERT(m_iMaxCount >= iCount);
  m_iCount = iCount;
}

template <class T, int G>
void CArray<T, G>::SetMaxCount(int iMaxCount)
{
  if (iMaxCount == m_iMaxCount)
    return;
  ASSERT(iMaxCount > 0);
  T *p = (T *) NEWARR(uint8_t, iMaxCount * sizeof(T));
  int iSize = Util::Min(iMaxCount, m_iCount);
  for (int i = 0; i < iSize; i++)
		TConDestructor<T>::ConstructCopy(p + i, m_pArray[i]);
	TConDestructor<T>::Destroy(m_pArray, m_iCount);
	DELARR(m_iMaxCount * sizeof(T), (uint8_t *) m_pArray);
  m_pArray = p;
	m_iCount = iSize;
  m_iMaxCount = iMaxCount;
}

template <class T, int G>
void CArray<T, G>::Append(T const &t)
{
  SetCount(m_iCount + 1);
  m_pArray[m_iCount - 1] = t;
}

// CSortedArray --------------------------------------------------------------------

template <class T, class K, class P, int G>
CSortedArray<T, K, P, G>::CSortedArray(int iMaxCount): CArray<T, G>(iMaxCount)
{
}

template <class T, class K, class P, int G>
int CSortedArray<T, K, P, G>::Add(T t)
{
  int i;
  SetCount(m_iCount + 1);
  i = m_iCount - 1;
  while (i > 0 && P::Lt(t, m_pArray[i - 1])) {
    m_pArray[i] = m_pArray[i - 1];
    i--;
  }
  m_pArray[i] = t;
  return i;
}

template <class T, class K, class P, int G>
T CSortedArray<T, K, P, G>::Remove(int i)
{
  ASSERT(i >= 0 && i < m_iCount);
  T t = m_pArray[i];
  while (i < m_iCount - 1) {
    m_pArray[i] = m_pArray[i + 1];
    i++;
  }
  SetCount(m_iCount - 1);
  return t;
}

template <class T, class K, class P, int G>
int CSortedArray<T, K, P, G>::RemoveValue(T t)
{
  int i, iPos;

  iPos = Find(t);
  if (iPos < 0)
    return iPos;
  for (i = iPos; i < m_iCount - 1; i++)
    m_pArray[i] = m_pArray[i + 1];
  SetCount(m_iCount - 1);
  return iPos;
}

template <class T, class K, class P, int G>
template <class K1>
int CSortedArray<T, K, P, G>::Find(K1 k) const
{
  // P is assumed to be the equivalent of < or >
  return Util::BinSearch<T*, K1, P>(m_pArray, m_iCount, k);
}

// CHeap ---------------------------------------------------------------------------

template <class T, class K, class P, int G>
CHeap<T, K, P, G>::CHeap(int iMaxCount): CArray<T, G>(iMaxCount)
{
}

template <class T, class K, class P, int G>
void CHeap<T, K, P, G>::InitHeap()
{
  for (int i = 1; i < m_iCount; i++)
    SiftUp(i);
}

template <class T, class K, class P, int G>
int CHeap<T, K, P, G>::Add(T t)
{
  Append(t);
  return SiftUp(m_iCount - 1);
}

template <class T, class K, class P, int G>
T CHeap<T, K, P, G>::Remove(int i)
{
  ASSERT(i >= 0 && i < m_iCount);
  T t = m_pArray[i];
  if (i == m_iCount - 1) {
    SetCount(m_iCount - 1);
    return t;
  }
  Util::Swap(m_pArray[i], m_pArray[m_iCount - 1]);
  SetCount(m_iCount - 1);
  Resort(i);
  return t;
}

template <class T, class K, class P, int G>
int CHeap<T, K, P, G>::RemoveValue(T t)
{
  int iInd = Find(t);
  if (iInd >= 0)
    Remove(iInd);
  return iInd;
}

template <class T, class K, class P, int G>
int CHeap<T, K, P, G>::Resort(int i)
{
  ASSERT(i >= 0 && i < m_iCount);
  int iInd;
  if (i > 0 && P::Lt(m_pArray[i], m_pArray[UpIndex(i)]))
    iInd = SiftUp(i);
  else
    iInd = SiftDown(i);
  return iInd;
}

template <class T, class K, class P, int G>
template <class K1>
int CHeap<T, K, P, G>::Find(K1 k, int iRoot) const
{
  int iRes, iDown;
  ASSERT(iRoot >= 0);
  if (iRoot >= m_iCount)
    return -1;
  if (P::Lt(k, m_pArray[iRoot]))
    return -1;
  if (P::Lt(m_pArray[iRoot], k)) {
    iDown = DownIndex(iRoot);
    iRes = Find(k, iDown);
    if (iRes < 0)
      iRes = Find(k, iDown + 1);
    return iRes;
  }
  return iRoot;
}

template <class T, class K, class P, int G>
int CHeap<T, K, P, G>::SiftUp(int i)
{
  ASSERT(i >= 0 && i < m_iCount);
  while (i) {
    int iUp = UpIndex(i);
    if (P::Lt(m_pArray[i], m_pArray[iUp]))
      Util::Swap(m_pArray[i], m_pArray[iUp]);
    else
      break;
    i = iUp;
  }
  return i;
}

template <class T, class K, class P, int G>
int CHeap<T, K, P, G>::SiftDown(int i)
{
  ASSERT(i >= 0 && i < m_iCount);
  int iNextInd = DownIndex(i);
  while (iNextInd < m_iCount) {
    if (iNextInd + 1 < m_iCount && P::Lt(m_pArray[iNextInd + 1], m_pArray[iNextInd]))
      iNextInd++;
    if (P::Lt(m_pArray[iNextInd], m_pArray[i]))
      Util::Swap(m_pArray[i], m_pArray[iNextInd]);
    else
      break;
    i = iNextInd;
    iNextInd = DownIndex(i);
  }
  return i;
}

#endif
