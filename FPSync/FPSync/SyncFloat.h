#pragma once

#include <intrin.h>

struct SyncFloat {
  using VecType = __m128;

  float m_value;

  inline SyncFloat(float value): m_value(value) {}
  inline SyncFloat(SyncFloat const &other) = default;
  inline SyncFloat(SyncFloat &&other) = default;
  inline SyncFloat() = default;

  operator float() const { return m_value; }

  inline SyncFloat &operator =(SyncFloat const &other) = default;
  inline bool operator ==(SyncFloat other) const { return m_value == other.m_value; }
  inline bool operator !=(SyncFloat other) const { return m_value != other.m_value; }
  inline bool operator <(SyncFloat other) const { return m_value < other.m_value; }
  inline bool operator <=(SyncFloat other) const { return m_value <= other.m_value; }
  inline bool operator >(SyncFloat other) const { return m_value > other.m_value; }
  inline bool operator >=(SyncFloat other) const { return m_value >= other.m_value; }

  static inline VecType AsVecType(float f)
  { 
    return _mm_load_ss(&f); 
  }

  static inline float FromVecType(VecType v) 
  { 
    float f;  
    _mm_store_ss(&f, v);
    return f;
  }

  inline SyncFloat operator -() const
  {
    return SyncFloat(0) - *this;
  }

  inline SyncFloat operator +(SyncFloat const &other) const
  {
    VecType v1 = AsVecType(m_value);
    VecType v2 = AsVecType(other.m_value);
    VecType res = _mm_add_ss(v1, v2);
    return SyncFloat(FromVecType(res));
  }

  inline SyncFloat operator -(SyncFloat const &other) const
  {
    VecType v1 = AsVecType(m_value);
    VecType v2 = AsVecType(other.m_value);
    VecType res = _mm_sub_ss(v1, v2);
    return SyncFloat(FromVecType(res));
  }

  inline SyncFloat operator *(SyncFloat const &other) const
  {
    VecType v1 = AsVecType(m_value);
    VecType v2 = AsVecType(other.m_value);
    VecType res = _mm_mul_ss(v1, v2);
    return SyncFloat(FromVecType(res));
  }

  inline SyncFloat operator /(SyncFloat const &other) const
  {
    VecType v1 = AsVecType(m_value);
    VecType v2 = AsVecType(other.m_value);
    VecType res = _mm_div_ss(v1, v2);
    return SyncFloat(FromVecType(res));
  }

  static float Test();
};