#ifndef __ALLOCATOR_H
#define __ALLOCATOR_H

#include <stdint.h>
#include <malloc.h>
#include "Base.h"
#include "../OS/Threads.h"

#if defined(_MSC_VER) || defined(__MINGW32__)
inline size_t malloc_usable_size(void *p) { return _msize(p); }
#endif

struct TMallocAllocator {
  inline void *Alloc(size_t uiSize, const char *pFile, int iLine) { return malloc(uiSize); }
  inline void Free(void *p, const char *pFile, int iLine)         { free(p); }
  inline size_t GetBlockSize(void *p)                             { return malloc_usable_size(p); }
  void Dump()                                                     {}
};

template <class A>
struct TDebugAllocator: public A {
  struct TBlockInfo {
    size_t           m_uiSize;
    const char      *m_pFile;
    int              m_iLine;
    TBlockInfo      *m_pPrev, *m_pNext;
    TDebugAllocator *m_pAllocator;
  };

  CLock                 m_kLock;
  TBlockInfo           *m_pBlocks;
  size_t                m_uiTotalSize;

  TDebugAllocator(): m_pBlocks(0), m_uiTotalSize(0) {}
  ~TDebugAllocator() { if (m_uiTotalSize) Dump(); ASSERT(m_uiTotalSize == 0); }

  inline void *Alloc(size_t uiSize, const char *pFile, int iLine)
  {
    CScopeLock lock(&m_kLock);
    TBlockInfo *pBlock = (TBlockInfo *) A::Alloc(uiSize + sizeof(TBlockInfo), pFile, iLine);
    pBlock->m_uiSize = uiSize;
    pBlock->m_pFile = pFile;
    pBlock->m_iLine = iLine;
    pBlock->m_pPrev = 0;
    if (m_pBlocks)
      m_pBlocks->m_pPrev = pBlock;
    pBlock->m_pNext = m_pBlocks;
    pBlock->m_pAllocator = this;
    m_pBlocks = pBlock;
    m_uiTotalSize += uiSize;
    return pBlock + 1;
  }

  inline void Free(void *p, const char *pFile, int iLine)
  {
    CScopeLock lock(&m_kLock);
    TBlockInfo *pBlock = (TBlockInfo *) p - 1;
    ASSERT(pBlock->m_pAllocator == this);
    m_uiTotalSize -= pBlock->m_uiSize;
    if (pBlock->m_pPrev)
      pBlock->m_pPrev->m_pNext = pBlock->m_pNext;
    else
      m_pBlocks = pBlock->m_pNext;
    if (pBlock->m_pNext)
      pBlock->m_pNext->m_pPrev = pBlock->m_pPrev;
    pBlock->m_pAllocator = 0;
    A::Free(pBlock, pFile, iLine);
  }

  inline size_t GetBlockSize(void *p)
  {
    CScopeLock lock(&m_kLock);
    TBlockInfo *pBlock = (TBlockInfo *) p - 1;
    return pBlock->m_uiSize;
  }

  void Dump()
  {
    CScopeLock lock(&m_kLock);
    size_t uiSize = 0;
    for (TBlockInfo *pBlock = m_pBlocks; pBlock; pBlock = pBlock->m_pNext) {
      fprintf(stderr, "Block start: %p, size: %zu, file: %s, line: %d\n", (pBlock + 1), pBlock->m_uiSize, pBlock->m_pFile, pBlock->m_iLine);
      uiSize += pBlock->m_uiSize;
    }
    ASSERT(uiSize == m_uiTotalSize);
    fprintf(stderr, "Total size: %zu\n", uiSize);
  }
};

template <class A>
struct TTrackingAllocator: public A {
  CAtomic<size_t> m_uiSize;

  TTrackingAllocator(): m_uiSize(0) {}
  ~TTrackingAllocator() {}

  inline void *Alloc(size_t uiSize, const char *pFile, int iLine)
  {
    void *p = A::Alloc(uiSize, pFile, iLine);
    m_uiSize += GetBlockSize(p);
    return p;
  }

  inline void Free(void *p, const char *pFile, int iLine)
  {
    m_uiSize -= GetBlockSize(p);
    A::Free(p, pFile, iLine);
  }

  void Dump() { fprintf(stderr, "Allocated memory: %d\n", (size_t) m_uiSize); A::Dump(); }

  using A::GetBlockSize;
};

#ifdef _DEBUG
typedef TTrackingAllocator<TDebugAllocator<TMallocAllocator> > TDefAllocator;
#else
typedef TMallocAllocator TDefAllocator;
#endif

template <class T>
struct TSpecifyAllocator {
  typedef TDefAllocator Type;
};

// Smart pointers
class CRefCount {
public:
	uint32_t m_dwCount;

	inline CRefCount()                           { m_dwCount = 0; }
	inline CRefCount(const CRefCount &kRefCount) { m_dwCount = 0; }
	inline ~CRefCount()                          { ASSERT(!m_dwCount); }

	inline uint32_t Get() const                  { return m_dwCount; }
	inline void     Inc()                        { ++m_dwCount; }
	inline void     Dec()                        { ASSERT(m_dwCount); --m_dwCount; }
};

#define DEFREFCOUNT                                                  \
  public:                                                            \
    mutable CRefCount m_RefCount;                                    \
    inline uint32_t GetRefCount() const { return m_RefCount.Get(); } \
    inline void     Acquire() const     { m_RefCount.Inc(); }        \
    inline void     Release() const     { m_RefCount.Dec(); if (!m_RefCount.Get()) DEL(this);  }

#define DEFREFCOUNT_DUMMY                             \
  public:                                             \
    inline uint32_t GetRefCount() const { return 0; } \
    inline void     Acquire() const {}                \
    inline void     Release() const { DEL(this); }

template <class T>
class CSmartPtr {
public:
  typedef T TElem;

	T *m_pPtr;

	explicit CSmartPtr(T *pPtr = 0):
	  m_pPtr(pPtr)                                          { Acquire(m_pPtr); }
  explicit CSmartPtr(const CSmartPtr &pPtr)               { m_pPtr = pPtr.m_pPtr; Acquire(m_pPtr); }
	~CSmartPtr()                                            { Release(m_pPtr); }

	static inline void Acquire(T *pPtr)                     { if (pPtr) pPtr->Acquire(); }
	static inline void Release(T *pPtr)                     { if (pPtr) pPtr->Release(); }

	inline operator T*()                                    { return m_pPtr; }
	inline T &operator *()                                  { return *m_pPtr; }
	inline T *operator ->()                                 { return m_pPtr; }
 	inline operator const T*() const                        { return m_pPtr; }
	inline const T &operator *() const                      { return *m_pPtr; }
	inline const T *operator ->() const                     { return m_pPtr; }

  inline CSmartPtr<T> &operator =(T *pPtr)                { Acquire(pPtr); Release(m_pPtr); m_pPtr = pPtr; return *this; }
  inline CSmartPtr<T> &operator =(const CSmartPtr &pPtr)  { return operator =(pPtr.m_pPtr); }
};

template <class T>
class CPtrDeleter {
public:
  static void FreePtr(T *pPtr) { DEL(pPtr); }
};

template <class T>
class CPtrReleaser {
public:
  static void FreePtr(T *pPtr) { if (pPtr) pPtr->Release(); }
};

template <class T, class D>
class CAutoFreePtr {
public:
  typedef T TElem;

	T *m_pPtr;

	explicit CAutoFreePtr(T *pPtr): m_pPtr(pPtr)     {}
	~CAutoFreePtr()                                  { ReleasePtr(); }

	inline operator T*()                             { return m_pPtr; }
	inline T &operator *()                           { return *m_pPtr; }
	inline T *operator ->()                          { return m_pPtr; }

  inline void ReleasePtr()                         { D::FreePtr(m_pPtr); m_pPtr = 0; }
};

template <class T>
class CAutoDeletePtr: public CAutoFreePtr<T, CPtrDeleter<T> > {
public:
  explicit CAutoDeletePtr(T *pPtr = 0): CAutoFreePtr<T, CPtrDeleter<T> >(pPtr) {}
};

template <class T>
class CAutoReleasePtr: public CAutoFreePtr<T, CPtrReleaser<T> > {
public:
  explicit CAutoReleasePtr(T *pPtr = 0): CAutoFreePtr<T, CPtrReleaser<T> >(pPtr) {}
};

#endif // __ALLOCATOR_H
