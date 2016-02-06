#include "stdafx.h"
#include "File.h"
#include <ctype.h>

CRTTIRegisterer<CFile> g_RegFile;
// CFile ---------------------------------------------------------

CFile::ERRCODE CFile::WriteBuf(void const *pBuf, int iBytes)
{
  ERRCODE err = Write(&iBytes, sizeof(iBytes));
  if (err || !iBytes)
    return err;
  err = Write(pBuf, iBytes);
  return err;
}

CFile::ERRCODE CFile::ReadBuf(void *&pBuf, int &iBytes)
{
  int iSize;
  ERRCODE err = Read(&iSize, sizeof(iSize));
  if (err)
    return err;
  ASSERT(!pBuf || iSize <= iBytes);
  if (pBuf && iSize > iBytes)
    return -1;
  iBytes = iSize;
  if (!iSize)
    return 0;
  if (!pBuf)
    pBuf = NEWARR(uint8_t, iSize);
  err = Read(pBuf, iSize);
  return err;
}

// CFileSystem ----------------------------------------------------------------

CRTTIRegisterer<CFileSystem> g_RegFileSystem;
CRTTIRegisterer<CFileSystem::CFileIter> g_RegFileSystemFileIter;

CFileSystem *CFileSystem::s_pFileSystem = 0;

void CFileSystem::Create()
{
#if defined(WINDOWS)
  char const *chFSClass = "CWinFileSystem";
#elif defined(LINUX)
  char const *chFSClass = "CLinFileSystem";
#endif // WINDOWS
  CRTTIHolder::Get()->Find(chFSClass)->CreateInstance();

  CFileSystem::Get()->Init();
}

void CFileSystem::Destroy()
{
  DEL(CFileSystem::Get());
}

CFileSystem::CFileSystem()
{
  s_pFileSystem = this;
}

CFileSystem::~CFileSystem()
{
}

void CFileSystem::Init()
{
  GetCurrentDirectory(m_sRootPath);
  m_sRootPath += CStrAny(ST_WHOLE, "/");
}

CFile *CFileSystem::OpenFile(CStrAny const &sFile, unsigned int uiFlags)
{
  CStrAny sPath = ResolvePath(sFile);
  CFile *pFile = (CFile *) GetFileRTTI()->CreateInstance();
  pFile->Init(sPath, uiFlags);
  if (!pFile->IsValid()) {
    DEL(pFile);
    pFile = 0;
  }
  return pFile;
}

CStrAny CFileSystem::ResolvePath(CStrAny const &sFile)
{
  CStrAny sPath(ST_WHOLE, ""), sTempDir, sRes;
  int i;
  bool bSkipSlash = false;
  for (i = 0; i < sFile.Length(); i++) {
    char ch = sFile[i];
    if (ch == '\\')
      ch = '/';
    if (ch != '/' || !bSkipSlash)
      sPath += CStrAny(ST_PART, &ch, 1);
    bSkipSlash = (ch == '/');
  }
  CStrAny sDrive, sDir, sName, sExt;
  ParsePath(sPath, &sDrive, &sDir, &sName, &sExt);

  CStrAny sRootDrive, sRootDir, sRootName, sRootExt;

  if (!sDrive || !sDir) {
    ParsePath(m_sRootPath, &sRootDrive, &sRootDir, &sRootName, &sRootExt);
    sRootDir += sRootName;
    sRootDir += sRootExt;
  }

  if (!sDrive)
    sDrive = sRootDrive;

  if (!sDir)
    sDir = sRootDir;

  if (!!sDir && sDir[0] != '/') { // relative path
    sTempDir = sRootDir;
    sTempDir += sDir;
    sTempDir += sName + sExt;
    int iPos, iPrev;
    while (1) {
      iPos = sTempDir.Find(CStrAny(ST_WHOLE, "/.."));
      if (iPos < 0)
        break;
      if (!iPos)
        sTempDir = CStrAny(ST_WHOLE, "/") + sTempDir.SubStr(3, sTempDir.Length());
      else {
        for (iPrev = iPos - 1; iPrev >= 0; iPrev--)
          if (sTempDir[iPrev] == '/')
            break;
        sTempDir = sTempDir.SubStr(0, iPrev + 1) + sTempDir.SubStr(iPos + 3, sTempDir.Length());
      }
    }
    sRes = sDrive;
    sRes += sTempDir;
  } else {
    sRes = sDrive;
    sRes += sDir;
    sRes += sName + sExt;
  }

  return sRes;
}

CStrAny CFileSystem::GetCurrentRoot()
{
  return m_sRootPath;
}

void CFileSystem::SetCurrentRoot(CStrAny const &sPath)
{
  m_sRootPath.Clear();
  CStrAny sPath1 = sPath;
  sPath1 += CStrAny(ST_WHOLE, "/");
  m_sRootPath = ResolvePath(sPath1);
}

CFileSystem::CFileIter *CFileSystem::GetFileIter(CStrAny const &sPath)
{
  CFileIter *pIter = (CFileIter *) GetFileIterRTTI()->CreateInstance();
  CStrAny sResPath = ResolvePath(sPath);
  *pIter = sResPath;
  return pIter;
}

bool CFileSystem::ParsePath(CStrAny const &sFile, CStrAny *pDrive, CStrAny *pPath, CStrAny *pName, CStrAny *pExtension)
{
  CStrAny sDrive, sPath, sName, sExt;
  int i;
  if (!pDrive)
    pDrive = &sDrive;
  else
    pDrive->Clear();
  if (!pPath)
    pPath = &sPath;
  else
    pPath->Clear();
  if (!pName)
    pName = &sName;
  else
    pName->Clear();
  if (!pExtension)
    pExtension = &sExt;
  else
    pExtension->Clear();
  if (sFile.Length() >= 2 && sFile[1] == ':' && isalpha(sFile[0]))
    *pDrive = sFile.SubStr(0, 2);
  for (i = sFile.Length() - 1; i >= pDrive->Length(); i--) {
    if (sFile[i] == '.' && !*pExtension)
      *pExtension = sFile.SubStr(i, sFile.Length());
    if (sFile[i] == '/' || sFile[i] == '\\') {
      *pPath = sFile.SubStr(pDrive->Length(), i + 1);
      break;
    }
  }
  *pName = sFile.SubStr(pDrive->Length() + pPath->Length(), sFile.Length() - pExtension->Length());

  return true;
}
