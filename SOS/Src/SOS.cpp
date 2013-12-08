// SOS.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <math.h>
#include "Frontend.h"
#include "File.h"

void TestFiles()
{
  CStrAny sFile(ST_STR, "FileTstvc.txt");
  CFile *pFile = CFileSystem::Get()->OpenFile(sFile, CFile::FOF_CREATE | CFile::FOF_READ | CFile::FOF_WRITE | CFile::FOF_TRUNCATE | CFile::FOF_TEXT);

  char chMsg[] = "This is a message from a developer\n\tHI!!1\nEnd of message\n";
  pFile->Write(chMsg, ARRSIZE(chMsg));
  delete pFile;

  CStrAny sDir(ST_STR, "Dir.vc");
  CFileSystem::Get()->CreateDirectory(sDir);

  CFileSystem::CFileIter *pIt = CFileSystem::Get()->GetFileIter(CStrAny(ST_STR, "*vc*"));

  while (*pIt) {
    CStrAny sName = pIt->GetName();
    CFile::FILESIZE iSize = pIt->GetSize();
    UINT uiAttr = pIt->GetAttributes();
    printf("%s %d %x\n", sName.m_pBuf, (int)iSize, uiAttr);
    ++(*pIt);
  }

  delete pIt;

  CFile::ERRCODE err = CFileSystem::Get()->DeleteDirectory(sDir);
  printf("Delete directiry error code: %d\n", err);

  pFile = CFileSystem::Get()->OpenFile(sFile, CFile::FOF_READ | CFile::FOF_TEXT);

  char ch;
  while (!pFile->Read(ch)) {
    printf("%c", ch);
  }

  delete pFile;

  err = CFileSystem::Get()->DeleteFile(sFile);
  printf("Delete file error code: %d\n", err);
}

int main(int argc, char* argv[])
{
  CFileSystem::Create();

//  TestFiles();

  int iRes;

	iRes = ProcessInput();

  CFileSystem::Destroy();

	return iRes;
}

