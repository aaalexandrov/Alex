// SOS.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <math.h>
#include "Frontend.h"
#include "File.h"

int main(int argc, char* argv[])
{
  CFileSystem::Create();

  int iRes = ProcessInput();

  CFileSystem::Destroy();

	return iRes;
}

