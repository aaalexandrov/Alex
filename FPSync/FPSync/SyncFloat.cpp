#include "pch.h"
#include "SyncFloat.h"

float SyncFloat::Test()
{
  SyncFloat one(1), zero(0);
  SyncFloat o = one + zero;
  SyncFloat two = o + one;
  SyncFloat four = two * two;
  SyncFloat minFour = -four;
  SyncFloat minTwo = minFour / two;
  return (float)minTwo;
}