#include "stdafx.h"
#include "Util.h"

int SolveQuadratic(float a, float b, float c, float & x0, float & x1)
{
  if (Eq(a, 0)) {
    if (Eq(b, 0)) {
      if (Eq(c, 0)) {
        x0 = 0;
        x1 = 1;
        // any x is a solution
        return std::numeric_limits<int>::max();
      }
      x0 = x1 = std::numeric_limits<float>::quiet_NaN();
      return 0;
    }
    x0 = x1 = -c / b;
    return 1;
  }

  float d = b * b - 4 * a * c;
  if (d < 0) {
    x0 = x1 = std::numeric_limits<float>::quiet_NaN();
    return 0;
  }

  d = std::sqrt(d);
  x0 = (-b - d) / (2 * a);
  x1 = (-b + d) / (2 * a);

  return Eq(x0, x1) ? 1 : 2;
}
