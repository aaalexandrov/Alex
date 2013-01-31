// SOS.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <math.h>
#include "Frontend.h"
#include <windows.h>
#include "File.h"


int _tmain(int argc, _TCHAR* argv[])
{
  new CFileSystem();

  int iRes;

	iRes = ProcessInput();

  delete CFileSystem::Get();

	return iRes;
}

