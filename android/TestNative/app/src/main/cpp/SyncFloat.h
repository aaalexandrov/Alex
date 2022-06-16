#pragma once

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <arm_neon.h>
#endif

struct SyncFloat {
#ifdef _MSC_VER
  using VecType = __m128;
#else
  using VecType = float32x2_t;
#endif

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
#ifdef _MSC_VER
    return _mm_load_ss(&f);
#else
    return vld1_dup_f32(&f);
#endif
  }

  static inline float FromVecType(VecType v) 
  { 
    float f;
#ifdef _MSC_VER
    _mm_store_ss(&f, v);
#else
    vst1_lane_f32(&f, v, 0);
#endif
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
#ifdef _MSC_VER
    VecType res = _mm_add_ss(v1, v2);
#else
    VecType res = vadd_f32(v1, v2);
#endif
    return SyncFloat(FromVecType(res));
  }

  inline SyncFloat operator -(SyncFloat const &other) const
  {
    VecType v1 = AsVecType(m_value);
    VecType v2 = AsVecType(other.m_value);
#ifdef _MSC_VER
    VecType res = _mm_sub_ss(v1, v2);
#else
    VecType res = vsub_f32(v1, v2);
#endif
    return SyncFloat(FromVecType(res));
  }

  inline SyncFloat operator *(SyncFloat const &other) const
  {
    VecType v1 = AsVecType(m_value);
    VecType v2 = AsVecType(other.m_value);
#ifdef _MSC_VER
    VecType res = _mm_mul_ss(v1, v2);
#else
    VecType res = vmul_f32(v1, v2);
#endif
    return SyncFloat(FromVecType(res));
  }

  inline SyncFloat operator /(SyncFloat const &other) const
  {
    VecType v1 = AsVecType(m_value);
    VecType v2 = AsVecType(other.m_value);
#ifdef _MSC_VER
    VecType res = _mm_div_ss(v1, v2);
    return SyncFloat(FromVecType(res));
#else
    //VecType res = vdiv_f32(v1, v2);
    float num = m_value;
    __asm__(
        "vldr.f32 s0, %[n]\n\t"
        "vldr.f32 s1, %[d]\n\t"
        "vdiv.f32 s0, s0, s1\n\t"
        "vstr.f32 s0, %[n]"
    : [n] "=rm" (num)
    : [d] "rm" (other.m_value)
    : "s0", "s1");
    return num;
#endif
  }

  static float Test();
};