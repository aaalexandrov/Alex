// SOS.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <math.h>
#include "Frontend.h"
#include "File.h"

void Test()
{
  float fFrac, fWhole;
  fFrac = modff(-1234.5678f, &fWhole);
  printf("%g %g\n", fWhole, fFrac);
}

int main(int argc, char* argv[])
{
  CFileSystem::Create();

  Test();

  int iRes = ProcessInput();

  CFileSystem::Destroy();

	return iRes;
}

