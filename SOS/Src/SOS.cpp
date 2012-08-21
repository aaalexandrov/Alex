// SOS.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <math.h>
#include "Token.h"
#include "Expression.h"
#include "Frontend.h"
#include <windows.h>
#include "Str.h"

void StrAnyTest()
{
  CStrAny s(ST_WHOLE, "Asdffgh");
  CStrAny s1;

  s1 = s;
  CStrAny s2(s + s1);

  CStrAny s3 = s >> 1;
  bool b = s == s1;
  bool c = s < s3;
  CStrAny s4(s3, ST_CONST);
  int d = s3.Cmp(s4);
  s = s4;
  int e = s3.Cmp(s);

  float f = - 4.0f/3 - 125;
  CStrAny sF(ST_STR, f);
  CStrAny sI(ST_STR, -2398474);
  CStrAny sM(ST_STR, (int) 0x80000000);
  CStrAny sH(ST_STR, (int) 0x347856438);
};

int _tmain(int argc, _TCHAR* argv[])
{
  StrAnyTest();

  int iRes;
  CInterpreter Interp;

	iRes = ProcessInput(Interp);

	return iRes;
}

