#pragma once

#include "namespace.h"

NAMESPACE_BEGIN(util)

template <class DURATION>
float ToSeconds(DURATION duration) 
{
  return std::chrono::duration_cast<std::chrono::duration<float>>(duration).count();
}

NAMESPACE_END(util)