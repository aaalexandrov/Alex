#ifndef __MEM_H
#define __MEM_H

#include <stdint.h>
#include <utility>

template <class A>
A &Instance()
{
  static A kInstance;
  return kInstance;
}

struct TAllocator {
  virtual void *Alloc(size_t uiSize) = 0;
  virtual void Free(void *p) = 0;
  virtual TAllocator &GetNested() = 0;

  static inline TAllocator *&Current();
};

struct TDefAllocator: public TAllocator {
  TAllocator *m_pNested;

  TDefAllocator(TAllocator *pNested = nullptr) { m_pNested = pNested ? pNested : this; }

  virtual void *Alloc(size_t uiSize)       { return new uint8_t[uiSize]; }
  virtual void Free(void *p)               { delete[] (uint8_t *) p; }
  virtual TAllocator &GetNested()          { return *m_pNested; }
};

#define DEF_ALLOC        (&Instance<TDefAllocator>())
#define CUR_ALLOC        TAllocator::Current()

// Template to get around the problem of passing a coma inside a single macro argument, as in Type<Param1, Param2>.
// Such a type must be enclosed in ID_TYPE inside the macro invocation, e.g. NEW(ID_TYPE(Type<Param1, Param2>), Alloc, 5)
template <class T> struct TArgGet;
template <class F, class P> struct TArgGet<F(P)> { typedef P Type; };

#define ID_TYPE(...) typename TArgGet<void(__VA_ARGS__)>::Type

/*
#define NEW_A(A, T, Args) new ((A).Alloc(sizeof(T))) T Args
#define DEL_A(A, P) DeleteImpl(A, 1, P)

#define NEWARR_A(A, T, C) new ((A).Alloc(sizeof(T) * (C))) T[C]
#define DELARR_A(A, C, P) DeleteImpl(A, C, P)
*/

// Macro to optionally insert a comma depending on whether its argument list is empty
#define INS_COMMA(...) GLUE_NAME(PREFIX_COMMA(__VA_ARGS__), _PRESENT)
#define PREFIX_COMMA(...) ARG_GLUE(GET_ARG6, (EMPTY_COMMA __VA_ARGS__ (), COMMA, COMMA, COMMA, COMMA, COMMA, NO_COMMA))
#define EMPTY_COMMA() ,,,,,NO_COMMA
#define COMMA_PRESENT ,
#define NO_COMMA_PRESENT

#define ID_ARG(...) __VA_ARGS__

#define NEW_A(A, T, Args) NewImpl<T>(A, 1 INS_COMMA Args ID_ARG Args)
#define DEL_A(A, P) DeleteImpl(A, 1, P)

#define NEWARR_A(A, T, C) NewImpl<T>(A, C)
#define DELARR_A(A, C, P) DeleteImpl(A, C, P)

#define NEW(T, Args) NEW_A(*CUR_ALLOC, T, Args)
#define DEL(P)       DEL_A(*CUR_ALLOC, P)

#define NEWARR(T, C) NEWARR_A(*CUR_ALLOC, T, C)
#define DELARR(C, P) DELARR_A(*CUR_ALLOC, C, P)

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
/*
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
*/

template <class T, class... Args>
T *NewImpl(TAllocator &kAlloc, size_t uiCount, Args&&... args)
{
  T *pT = (T *) kAlloc.Alloc(sizeof(T) * uiCount);
  TAllocator *pPrevious = TAllocator::Current();
  TAllocator::Current() = &kAlloc.GetNested();
  for (size_t i = 0; i < uiCount; ++i)
    new (pT + i) T(std::forward<Args>(args)...);
  TAllocator::Current() = pPrevious;
  return pT;
}

template <class T>
void DeleteImpl(TAllocator &kAlloc, size_t uiCount, T *pT)
{
  if (pT) {
    TAllocator *pPrevious = TAllocator::Current();
    TAllocator::Current() = &kAlloc.GetNested();
    for (size_t i = 0; i < uiCount; ++i)
      pT[i].~T();
    TAllocator::Current() = pPrevious;
    kAlloc.Free((void *) pT);
  }
}

inline TAllocator *&TAllocator::Current()
{
  thread_local TAllocator *pCurrent = DEF_ALLOC;
  return pCurrent;
}

#endif // __MEM_H
