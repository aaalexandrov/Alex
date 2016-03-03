#ifndef __HASHINNER_H
#define __HASHINNER_H

#include "Array.h"
#include "Util.h"

template <class T, class K = T, class H = Util::HashSize_T, class P = Util::Equal<T> >
class CHashInner {
public:
  const static int INITIAL_SIZE = 15;

  typedef T Elem;

	class CTableElem {
	public:
		uint8_t m_btData[sizeof(T)];
		CTableElem *m_pNext;

		CTableElem()  { m_pNext = GetFreePtr(); }
		~CTableElem() { TConDestructor<T>::Destroy(&GetData()); }

		T &GetData()             { return *(T *) m_btData; }
		T const &GetData() const { return *(T *) m_btData; }

		bool IsFree() const { return m_pNext == GetFreePtr(); }
		void Clear() { if (!IsFree()) { TConDestructor<T>::Destroy(&GetData()); m_pNext = GetFreePtr(); } }

		void SetData(T *pData, CTableElem *pNext);
		void DeleteData() { if (!IsFree()) { DEL(GetData()); TConDestructor<T>::Destroy(&GetData()); m_pNext = GetFreePtr(); } }

		static CTableElem *GetFreePtr() { return 0; } // Pointer to mark a free element node
		static CTableElem *GetEndPtr()  { static CTableElem kEndElem; return &kEndElem; } // Pointer to mark an end of a hash bucket chain
	};

	class TIter {
	public:
		CHashInner const *m_pHash;
		int m_iIndex;

    TIter(CHashInner const *pHash = 0, int iDirection = 1);
    TIter(TIter const &it);

    void Init(CHashInner const *pHash, int iDirection);

    TIter &operator =(CHashInner const &hash);
		// TODO: Fix insertion and iteration order to work in the case of multihash (i.e. values with the same key are iterated in sequence)
    TIter &operator ++();
    TIter &operator --();
    operator bool () const;
    T &operator *();
    T &operator ->();
    T const &operator ->() const;
	};

public:
	CTableElem *m_pElements;
	int m_iCount, m_iMaxCount;
	int m_iLastFree;

	CHashInner(int iSize = 0);
	~CHashInner();

  void DeleteAll(int iSize = 0);
  void Clear(int iSize = 0);

  void Add(T t);
  void AddUnique(T t);
  void Remove(TIter it);
  bool RemoveValue(T t);
	template <class K1>
	TIter Find(K1 k) const;
  TIter Find(K k) const { return Find<K>(k); }

public:
	void Init(int iSize);

  int GetNextSize(int iSize);
	void Resize(int iSize);

	int GetFreeIndex();
};

template <class K, class V, class H = Util::HashSize_T, class P = Util::Equal<K> >
class CHashInnerKV: public CHashInner<Util::TKeyValue<K, V, H, P>, K, Util::TKeyValue<K, V, H, P>, Util::TKeyValue<K, V, H, P> > {
public:
  CHashInnerKV(int iSize = 0): CHashInner<Util::TKeyValue<K, V, H, P>, K, Util::TKeyValue<K, V, H, P>, Util::TKeyValue<K, V, H, P> >(iSize) {}
};

// Implementation ------------------------------------------------------------------

// CHashInner::CTableElem ----------------------------------------------------------

template <class T, class K, class H, class P>
void CHashInner<T, K, H, P>::CTableElem::SetData(T *pData, CTableElem *pNext)
{
	if (IsFree()) {
		if (pData) {
			TConDestructor<T>::ConstructCopy(&GetData(), *pData);
			m_pNext = pNext;
		}
	} else {
		if (pData)
			GetData() = *pData;
		else {
			TConDestructor<T>::Destroy(&GetData());
			m_pNext = GetFreePtr();
		}
	}
}

// CHashInner::TIter ---------------------------------------------------------------

template <class T, class K, class H, class P>
CHashInner<T, K, H, P>::TIter::TIter(CHashInner const *pHash, int iDirection)
{
  Init(pHash, iDirection);
}

template <class T, class K, class H, class P>
CHashInner<T, K, H, P>::TIter::TIter(TIter const &it)
{
  m_pHash = it.m_pHash;
  m_iIndex = it.m_iIndex;
}

template <class T, class K, class H, class P>
void CHashInner<T, K, H, P>::TIter::Init(CHashInner const *pHash, int iDirection)
{
  m_pHash = pHash;
  if (pHash) {
    if (iDirection > 0) {
			for (m_iIndex = 0; m_iIndex < m_pHash->m_iMaxCount && m_pHash->m_pElements[m_iIndex].IsFree(); m_iIndex++);
		} else
      if (iDirection < 0) {
				for (m_iIndex = m_pHash->m_iMaxCount - 1; m_iIndex >= 0 && m_pHash->m_pElements[m_iIndex].IsFree(); m_iIndex--);
			} else {
				m_iIndex = -1;
			}
  } else {
		m_iIndex = -1;
  }
}

template <class T, class K, class H, class P>
typename CHashInner<T, K, H, P>::TIter &CHashInner<T, K, H, P>::TIter::operator =(CHashInner const &hash)
{
  Init(&hash, 1);
  return *this;
}

template <class T, class K, class H, class P>
typename CHashInner<T, K, H, P>::TIter &CHashInner<T, K, H, P>::TIter::operator ++()
{
	ASSERT(m_pHash && m_iIndex >= -1 && m_iIndex < m_pHash->m_iMaxCount);
	for (m_iIndex++; m_iIndex < m_pHash->m_iMaxCount && m_pHash->m_pElements[m_iIndex].IsFree();	m_iIndex++);
  return *this;
}

template <class T, class K, class H, class P>
typename CHashInner<T, K, H, P>::TIter &CHashInner<T, K, H, P>::TIter::operator --()
{
	ASSERT(m_pHash && m_iIndex >= 0 && m_iIndex <= m_pHash->m_iMaxCount);
	for (m_iIndex--; m_iIndex >= 0 && m_pHash->m_pElements[m_iIndex].IsFree(); m_iIndex--);
	return *this;
}

template <class T, class K, class H, class P>
CHashInner<T, K, H, P>::TIter::operator bool () const
{
	return m_pHash && m_iIndex >= 0 && m_iIndex < m_pHash->m_iMaxCount;
}

template <class T, class K, class H, class P>
T &CHashInner<T, K, H, P>::TIter::operator *()
{
  ASSERT(m_pHash && m_iIndex >= 0 && m_iIndex < m_pHash->m_iMaxCount);
	ASSERT(!m_pHash->m_pElements[m_iIndex].IsFree());
  return m_pHash->m_pElements[m_iIndex].GetData();
}

template <class T, class K, class H, class P>
T &CHashInner<T, K, H, P>::TIter::operator ->()
{
  ASSERT(m_pHash && m_iIndex >= 0 && m_iIndex < m_pHash->m_iMaxCount);
	ASSERT(!m_pHash->m_pElements[m_iIndex].IsFree());
  return m_pHash->m_pElements[m_iIndex].GetData();
}

template <class T, class K, class H, class P>
T const &CHashInner<T, K, H, P>::TIter::operator ->() const
{
  ASSERT(m_pHash && m_iIndex >= 0 && m_iIndex < m_pHash->m_iMaxCount);
	ASSERT(!m_pHash->m_pElements[m_iIndex].IsFree());
  return m_pHash->m_pElements[m_iIndex].GetData();
}

// CHashInner ----------------------------------------------------------------------

template <class T, class K, class H, class P>
CHashInner<T, K, H, P>::CHashInner(int iSize)
{
	if (!iSize)
		iSize = INITIAL_SIZE;
	Init(iSize);
}

template <class T, class K, class H, class P>
CHashInner<T, K, H, P>::~CHashInner()
{
  DELARR(m_iMaxCount, m_pElements);
}

template <class T, class K, class H, class P>
void CHashInner<T, K, H, P>::Init(int iSize)
{
	ASSERT(iSize > 0);
	m_iCount = 0;
	m_iMaxCount = iSize;
	m_iLastFree = m_iMaxCount - 1;
	m_pElements = NEWARR(CTableElem, m_iMaxCount);
}

template <class T, class K, class H, class P>
void CHashInner<T, K, H, P>::DeleteAll(int iSize)
{
  if (!iSize)
		iSize = m_iMaxCount;
	if (iSize != m_iMaxCount) {
		for (int i = 0; i < m_iMaxCount; i++)
			m_pElements[i].DeleteData();
    DELARR(m_iMaxCount, m_pElements);
		Init(iSize);
	} else {
		for (int i = 0; i < m_iMaxCount; i++)
			m_pElements[i].DeleteData();
		m_iCount = 0;
		m_iLastFree = m_iMaxCount - 1;
	}
}

template <class T, class K, class H, class P>
void CHashInner<T, K, H, P>::Clear(int iSize)
{
  if (!iSize)
		iSize = m_iMaxCount;
	if (iSize != m_iMaxCount) {
		DELARR(m_iMaxCount, m_pElements);
		Init(iSize);
	} else {
		for (int i = 0; i < m_iMaxCount; i++) {
			m_pElements[i].SetData(0, 0);
			m_pElements[i].m_iNext = i + 1;
		}
		m_iCount = 0;
		m_iLastFree = m_iMaxCount - 1;
	}
}

template <class T, class K, class H, class P>
int CHashInner<T, K, H, P>::GetNextSize(int iSize)
{
	if (iSize <= 0)
		iSize = 1;
	return Util::RoundUpToPow2Minus1(iSize);
}

template <class T, class K, class H, class P>
void CHashInner<T, K, H, P>::Resize(int iSize)
{
  if (iSize < m_iCount)
		iSize = m_iCount;
	if (iSize == m_iMaxCount)
		return;

  CHashInner<T, K, H, P> hash(iSize);
  Util::Swap(hash.m_pElements, m_pElements);
  Util::Swap(hash.m_iCount, m_iCount);
  Util::Swap(hash.m_iMaxCount, m_iMaxCount);
  Util::Swap(hash.m_iLastFree, m_iLastFree);

	for (int i = 0; i < hash.m_iMaxCount; i++)
		if (!hash.m_pElements[i].IsFree())
			Add(hash.m_pElements[i].GetData());
}

template <class T, class K, class H, class P>
int CHashInner<T, K, H, P>::GetFreeIndex()
{
	while (m_iLastFree >= 0 && !m_pElements[m_iLastFree].IsFree())
		m_iLastFree--;
	return m_iLastFree;
}

template <class T, class K, class H, class P>
void CHashInner<T, K, H, P>::Add(T t)
{
	if (m_iCount >= m_iMaxCount)
		Resize(GetNextSize(m_iMaxCount + 1));
  size_t uiHash = H::Hash(t) % m_iMaxCount;
	CTableElem *pPos = m_pElements + uiHash;
	if (pPos->IsFree()) {
		pPos->SetData(&t, CTableElem::GetEndPtr());
	} else {
		size_t uiHashFirst = H::Hash(pPos->GetData()) % m_iMaxCount;
		if (uiHashFirst != uiHash) { // Start of bucket for this hash is occupied by an element from a different bucket - relocate it
			CTableElem *pPrev = m_pElements + uiHashFirst;
			while (pPrev->m_pNext != pPos)
				pPrev = pPrev->m_pNext;
			int iFree = GetFreeIndex();
			ASSERT(iFree >= 0 && iFree < m_iMaxCount);
			m_pElements[iFree].SetData(&pPos->GetData(), pPos->m_pNext);
			pPrev->m_pNext = m_pElements + iFree;
			pPos->SetData(&t, CTableElem::GetEndPtr());
		} else { // There are elements in this bucket, link to the chain after the first
			int iFree = GetFreeIndex();
			m_pElements[iFree].SetData(&t, pPos->m_pNext);
			pPos->m_pNext = m_pElements + iFree;
		}
	}
	m_iCount++;
}

template <class T, class K, class H, class P>
void CHashInner<T, K, H, P>::AddUnique(T t)
{
	TIter it = Find(t);
	if (it)
		*it = t;
	else
		Add(t);
}

template <class T, class K, class H, class P>
void CHashInner<T, K, H, P>::Remove(TIter it)
{
  ASSERT(it.m_pHash == this);
  if (!it)
    return;
	CTableElem *pElem = m_pElements + it.m_iIndex;
	size_t uiHash = H::Hash(pElem->GetData()) % m_iMaxCount;
	if (uiHash == it.m_iIndex) { // Removing the first element of a bucket
		if (pElem->m_pNext != CTableElem::GetEndPtr()) { // Bucket has more than one element - move next to the starting index
			CTableElem *pNext = pElem->m_pNext;
			pElem->SetData(&pNext->GetData(), pNext->m_pNext);
			pElem = pNext;
		}
	} else { // Not a starting element for the bucket
		CTableElem *pPrev = m_pElements + uiHash;
		while (pPrev->m_pNext != pElem)
			pPrev = pPrev->m_pNext;
		pPrev->m_pNext = pElem->m_pNext;
	}
	pElem->SetData(0, 0);
	if (pElem - m_pElements > m_iLastFree)
		m_iLastFree = pElem - m_pElements;
  m_iCount--;
}

template <class T, class K, class H, class P>
bool CHashInner<T, K, H, P>::RemoveValue(T t)
{
  TIter it = Find(t);
  Remove(it);
  return !!it;
}

template <class T, class K, class H, class P>
template <class K1>
typename CHashInner<T, K, H, P>::TIter CHashInner<T, K, H, P>::Find(K1 k) const
{
  size_t uiHash = H::Hash(k) % m_iMaxCount;
	for (CTableElem *pElem = m_pElements + uiHash; !pElem->IsFree() && pElem != CTableElem::GetEndPtr(); pElem = pElem->m_pNext)
		if (P::Eq(pElem->GetData(), k)) {
			TIter it;
			it.m_pHash = this;
			it.m_iIndex = pElem - m_pElements;
			return it;
		}
  return TIter();
}

#endif
