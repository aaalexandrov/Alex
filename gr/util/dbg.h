#pragma once

#include <iostream>

namespace util {

#define ASSERT assert

#if defined(_DEBUG)
#define LOG(...) util::LogLine(std::cerr, __VA_ARGS__)
#else
#define LOG(...)
#endif

template <typename ARG, typename... ARGS>
void LogLine(std::ostream &out, ARG&& arg, ARGS&&... args) 
{
  out << std::forward<ARG>(arg);
  using expander = int[];
  static_cast<void>(expander{ 0, (void(out << std::forward<ARGS>(args)), 0)... });
  out << std::endl;
}

}