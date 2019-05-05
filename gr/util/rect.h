#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "mathutl.h"
#include "namespace.h"

NAMESPACE_BEGIN(util)

template <class NUM, int DIM>
class Box {
public:
  typedef glm::vec<DIM, NUM, glm::defaultp> Vec;

  template<bool IS_INTEGRAL>
  struct SizeOffsetImpl;

  template<>
  struct SizeOffsetImpl<false> {
    inline static Vec GetOffset() { return glm::zero<Vec>(); }
  };

  template<>
  struct SizeOffsetImpl<true> {
    inline static Vec GetOffset() { return glm::one<Vec>(); }
  };

  typedef SizeOffsetImpl<std::is_integral<NUM>::value> SizeOffset;

  Vec _min = Vec(util::limits<NUM>::Max());
  Vec _max = Vec(util::limits<NUM>::Min());

  inline Vec GetSize() const { return _max - _min + SizeOffset::GetOffset(); }
  inline void SetSize(Vec const &size) { _max = _min + size - SizeOffset::GetOffset(); }

  inline bool IsEmpty() const 
  {
    for (int d = 0; d < DIM; ++d) {
      if (_min[d] > _max[d])
        return true;
    }
    return false; 
  }

  bool operator==(Box const &other)
  {
    return _min == other._min && _max == other._max;
  }

  bool operator!=(Box const &other)
  {
    return !operator==(other);
  }
};


typedef Box<int32_t, 2> RectI;
typedef Box<float, 2>   RectF;

typedef Box<float, 3>   BoxF;

NAMESPACE_END(util)