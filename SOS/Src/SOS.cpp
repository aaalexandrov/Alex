// SOS.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <math.h>
#include "Token.h"
#include "Expression.h"
#include "Frontend.h"
#include <windows.h>
#include "Str.h"
#include "Encode.h"
#include "File.h"

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

void RangeEncoderTest()
{
  static const CStrAny sFile(ST_WHOLE, "/alex/evejew/blueprints.csv");
  //static const CStrAny sFile(ST_WHOLE, "/alex/tetris.pl");
  CFileBase *pFile = CFileSystem::Get()->OpenFile(sFile, CFileBase::FOF_READ);
  CArray<BYTE> arrInput;
  arrInput.SetCount((int) pFile->GetSize());
  pFile->Read(arrInput.m_pArray, arrInput.m_iCount);
  delete pFile;
  //memset(arrInput.m_pArray, 0, arrInput.m_iCount);
  //arrInput[0] = 1;
  CRangeEncoder encoder;
  encoder.Encode(arrInput.m_pArray, arrInput.m_iCount);
  CArray<BYTE> arrEncoded;
  arrEncoded.SetCount(encoder.m_arrOutput.m_iCount);
  memcpy(arrEncoded.m_pArray, encoder.m_arrOutput.m_pArray, arrEncoded.m_iCount);
  encoder.Decode(arrEncoded.m_pArray, arrEncoded.m_iCount);
  ASSERT(encoder.m_arrOutput.m_iCount == arrInput.m_iCount && !memcmp(encoder.m_arrOutput.m_pArray, arrInput.m_pArray, arrInput.m_iCount));
  float fEntropy = encoder.CalcEntropy();
  printf("Size: %d, encoded: %d, entropy: %g, information content: %g\n", arrInput.m_iCount, arrEncoded.m_iCount, fEntropy, fEntropy * arrInput.m_iCount / 8.0f);
}

int _tmain(int argc, _TCHAR* argv[])
{
  new CFileSystem();

  RangeEncoderTest();
  StrAnyTest();

  int iRes;
  CInterpreter Interp;

	iRes = ProcessInput(Interp);

  delete CFileSystem::Get();

	return iRes;
}

