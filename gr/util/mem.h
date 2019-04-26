#pragma once

#include <malloc.h>

namespace util {

inline size_t MemSize(void *mem)
{
  if (!mem)
    return 0;
  size_t size;
#if defined(_WIN32)
  size = _msize(mem);
#else
  size = malloc_size(mem);
#endif
  return size;
}

}