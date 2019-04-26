#pragma once

namespace util {

template <class DURATION>
float ToSeconds(DURATION duration) 
{
  return std::chrono::duration_cast<std::chrono::duration<float>>(duration).count();
}

}