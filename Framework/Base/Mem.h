#ifndef __MEM_H
#define __MEM_H

#include <stdint.h>
#include <atomic>
#include <malloc.h>
#include "Debug.h"

template <class A>
A &Instance()
{
  static A kInstance;
  return kInstance;
}

#if defined(_MSC_VER) || defined(__MINGW32__)
inline size_t malloc_usable_size(void *p) { return _msize(p); }
#endif

struct TMallocAllocator {
  inline void *Alloc(size_t uiSize, const char *pFile, int iLine) { return malloc(uiSize); }
  inline void Free(void *p, const char *pFile, int iLine)         { free(p); }
  inline size_t GetBlockSize(void *p)                             { return malloc_usable_size(p); }
};

template <class A>
struct TDebugAllocator: public A {
  struct TBlockInfo {
    size_t      m_uiSize;
    const char *m_pFile;
    int         m_iLine;
  };

  inline void *Alloc(size_t uiSize, const char *pFile, int iLine)
  {
    TBlockInfo *pBlock = (TBlockInfo *) A::Alloc(uiSize + sizeof(TBlockInfo), pFile, iLine);
    pBlock->m_uiSize = uiSize;
    pBlock->m_pFile = pFile;
    pBlock->m_iLine = iLine;
    return pBlock + 1;
  }

  inline void Free(void *p, const char *pFile, int iLine)
  {
    TBlockInfo *pBlock = (TBlockInfo *) p - 1;
    A::Free(pBlock, pFile, iLine);
  }

  inline size_t GetBlockSize(void *p)
  {
    TBlockInfo *pBlock = (TBlockInfo *) p - 1;
    return pBlock->m_uiSize;
  }
};

template <class A>
struct TTrackingAllocator: public A {
  std::atomic<size_t> m_uiSize;

  TTrackingAllocator(): m_uiSize(0) {}
  ~TTrackingAllocator() { ASSERT(m_uiSize == 0); }

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

  using A::GetBlockSize;
};

#ifdef _DEBUG
typedef TTrackingAllocator<TDebugAllocator<TMallocAllocator> > TDefAllocator;
#else
typedef TMallocAllocator TDefAllocator;
#endif

template <class T>
struct TGetAllocator {
  typedef TDefAllocator Type;
};

template <class T>
typename TGetAllocator<T>::Type& GetAllocatorInstance(T*) { return Instance<typename TGetAllocator<T>::Type>(); }

// Template to get around the problem of passing a coma inside a single macro argument, as in Type<Param1, Param2>.
// Such a type must be enclosed in ID_TYPE inside the macro invocation, e.g. NEW(ID_TYPE(Type<Param1, Param2>), Alloc, 5)
template <class T> struct TArgGet;
template <class F, class P> struct TArgGet<F(P)> { typedef P Type; };

#define ID_TYPE(...) typename TArgGet<void(__VA_ARGS__)>::Type

#define NEW_A(A, T, Args) new ((A).Alloc(sizeof(T), __FILE__, __LINE__)) T Args
#define DEL_A(A, P) DeleteImpl(A, 1, P, __FILE__, __LINE__)

#define NEWARR_A(A, T, C) new ((A).Alloc(sizeof(T) * (C), __FILE__, __LINE__)) T[C]
#define DELARR_A(A, C, P) DeleteImpl((A), C, P, __FILE__, __LINE__)

#define NEW(T, Args) NEW_A(GetAllocatorInstance((T*) 0), T, Args)
#define DEL(P)       DEL_A(GetAllocatorInstance(P), P)

#define NEWARR(T, C) NEWARR_A(GetAllocatorInstance((T*) 0), T, C)
#define DELARR(C, P) DELARR_A(GetAllocatorInstance(P), C, P)

// Allocate T in the allocator that's normally used for allocating type AT
#define NEW_T(AT, T, Args) NEW_A(GetAllocatorInstance((AT*) 0), T, Args)
#define DEL_T(AT, P)       DEL_A(GetAllocatorInstance((AT*) 0), P)

#define NEWARR_T(AT, T, C) NEWARR_A(GetAllocatorInstance((AT*) 0), T, C)
#define DELARR_T(AT, C, P) DELARR_A(GetAllocatorInstance((AT*) 0), C, P)


/*
// Macro to optionally insert a comma depending on whether its argument list is empty
#define INS_COMMA(...) GLUE_NAME(PREFIX_COMMA(__VA_ARGS__), _PRESENT)
#define PREFIX_COMMA(...) ARG_GLUE(GET_ARG6, (EMPTY_COMMA __VA_ARGS__ (), COMMA, COMMA, COMMA, COMMA, COMMA, NO_COMMA))
#define EMPTY_COMMA() ,,,,,NO_COMMA
#define COMMA_PRESENT ,
#define NO_COMMA_PRESENT

#define CAT_ARGS(...) INS_COMMA(__VA_ARGS__) __VA_ARGS__

#define NEW_A(A, T, Args) NewImpl<T>(A, 1 CAT_ARGS Args)
#define DEL_A(A, P) DeleteImpl(A, 1, P)

#define NEWARR_A(A, T, C) NewImpl<T>(A, C)
#define DELARR_A(A, C, P) DeleteImpl(A, C, P)
*/

/*
// Insane macro implementation to dispatch to the appropriate macro depending on the number of actual parameters supplied

// The following macro relies in the fact that a function macro will not be expanded if it's not followed immediately with parenthesized argument list
// EMPTY_ARGS is only expanded if the __VA_ARGS__ that's inserted before its parentheses is empty
// This is used to catch the 0 argument case that otherwise can't be distinguished from the 1 argument
#define NARGS(...) ARG_GLUE(GET_ARG6, (EMPTY_ARGS __VA_ARGS__ (), 5, 4, 3, 2, 1, 0))
#define EMPTY_ARGS() ,,,,,0
#define GET_ARG6(A1, A2, A3, A4, A5, A6, ...) A6
#define ARG_GLUE(X, Y) X Y

#define GLUE_NAME2(P, S) P ## S
#define GLUE_NAME1(P, S) GLUE_NAME2(P, S)
#define GLUE_NAME(P, S) GLUE_NAME1(P, S)

#define __NEW3    NEW_A
#define __NEW2    NEW

#define __DELETE2 DEL_A
#define __DELETE1 DEL

#define __NEWARR3 NEWARR_A
#define __NEWARR2 NEWARR

#define __DELARR3 DELARR_A
#define __DELARR2 DELARR

#define __NEW(...)    ARG_GLUE(GLUE_NAME(__NEW,    NARGS(__VA_ARGS__)), (__VA_ARGS__))
#define __DELETE(...) ARG_GLUE(GLUE_NAME(__DELETE, NARGS(__VA_ARGS__)), (__VA_ARGS__))
#define __NEWARR(...) ARG_GLUE(GLUE_NAME(__NEWARR, NARGS(__VA_ARGS__)), (__VA_ARGS__))
#define __DELARR(...) ARG_GLUE(GLUE_NAME(__DELARR, NARGS(__VA_ARGS__)), (__VA_ARGS__))

template <class T, class A, class... Args>
T *NewImpl(A &kAlloc, size_t uiCount, Args&&... args)
{
  T *pT = (T *) kAlloc.Alloc(sizeof(T) * uiCount);
  for (size_t i = 0; i < uiCount; ++i)
    new (pT + i) T(std::forward<Args>(args)...);
  return pT;
}
*/

template <class T, class A>
void DeleteImpl(A &kAlloc, size_t uiCount, T *pT, const char *pFile, int iLine)
{
  if (pT) {
    for (size_t i = 0; i < uiCount; ++i)
      pT[i].~T();
    kAlloc.Free((void *) pT, pFile, iLine);
  }
}

#endif // __MEM_H
