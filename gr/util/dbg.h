#pragma once

#include <iostream>
#include "namespace.h"

NAMESPACE_BEGIN(util)

#if defined(NDEBUG)
#define ASSERT(expression) ((void)0)
#else
#define ASSERT(expression) (void) ((!!(expression)) || (util::AssertFailed(#expression, __FILE__, (unsigned)__LINE__), 0))
#endif

void AssertFailed(char const *message, char const *file, unsigned line);

#if defined(NDEBUG)
#define LOG(...) ((void)0)
#else
#define LOG(...) util::LogLine(std::cerr, __VA_ARGS__)
#endif


template <typename ARG, typename... ARGS>
void LogLine(std::ostream &out, ARG&& arg, ARGS&&... args) 
{
  out << std::forward<ARG>(arg);
  using expander = int[];
  static_cast<void>(expander{ 0, (void(out << std::forward<ARGS>(args)), 0)... });
  out << std::endl;
}

NAMESPACE_END(util)