// FPSync.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include "SyncFloat.h"

int main()
{
  float f = SyncFloat::Test();
  std::cout << "Hello World! " << f << std::endl; 
}
