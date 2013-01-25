#ifndef __DEBUG_H
#define __DEBUG_H

void DbgPrint(const char *pFormat, ...);
void Assert(bool bCondition, const char *pCondition, const char *pFile, int nLine);

#ifdef _DEBUG

#define PRINT(...) DbgPrint(__VA_ARGS__)
#define ASSERT(cond) { bool bAssertCond = !!(cond); if (!bAssertCond) Assert(bAssertCond, #cond, __FILE__, __LINE__); }

#else

#define PRINT(...)   (void) 0
#define ASSERT(cond) (void) 0

#endif

//#define COMPILE_ASSERT(c) { typedef int Compile_Assert[1 - 2*!(c)]; }
//#define COMPILE_ASSERT(c) { enum { Compile_Assert = 1 / (int) !!(c) }; }

// Macros to ensure __LINE__ expansion before it gets concatenated
#define JOIN(X, Y)  JOIN2(X, Y)
#define JOIN2(X, Y) X##Y

#define COMPILE_ASSERT(c) enum { JOIN(Compile_Assert, __LINE__) = 1 / (int) !!(c) }

#endif
