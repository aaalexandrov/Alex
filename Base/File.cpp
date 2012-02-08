#include "stdafx.h"
#include "File.h"

#include <io.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <direct.h>
#include <ctype.h>
#include <stdio.h>

IMPRTTI_NOCREATE(CFileBase, CObject)
// CFileBase ---------------------------------------------------------

CFileBase::ERRCODE CFileBase::WriteBuf(void const *pBuf, int iBytes)
{
  ERRCODE err = Write(&iBytes, sizeof(iBytes));
  if (err || !iBytes)
    return err;
  err = Write(pBuf, iBytes);
  return err;
}

CFileBase::ERRCODE CFileBase::ReadBuf(void *&pBuf, int &iBytes)
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
    pBuf = new BYTE[iSize];
  err = Read(pBuf, iSize);
  return err;
}

// CFile -------------------------------------------------------------
IMPRTTI_NOCREATE(CFile, CFileBase)

CFile::CFile(const CStrBase &sName, unsigned int uiFlags)
{
  int iOpenFlags = 0, iCreate = 0;
  m_sFileName = sName;
  if (uiFlags & FOF_READ) {
    iCreate |= _S_IREAD;
    if (uiFlags & FOF_WRITE) {
      iOpenFlags |= _O_RDWR;
      iCreate |= _S_IWRITE;
    } else
      iOpenFlags |= _O_RDONLY;
  } else
    if (uiFlags & FOF_WRITE) {
      iOpenFlags |= _O_WRONLY;
      iCreate |= _S_IWRITE;
    }
  if (uiFlags & FOF_CREATE)
    iOpenFlags |= _O_CREAT;
  if (uiFlags & FOF_APPEND)
    iOpenFlags |= _O_APPEND;
  if (uiFlags & FOF_TRUNCATE)
    iOpenFlags |= _O_TRUNC;
  if (uiFlags & FOF_TEXT)
    iOpenFlags |= _O_TEXT;
  else
    iOpenFlags |= _O_BINARY;
  m_hFile = _open(sName, iOpenFlags, iCreate);
}

CFile::~CFile()
{
  if (m_hFile > 0)
    _close(m_hFile);
}

CFile::FILESIZE CFile::GetSize() const
{
  ASSERT(IsValid());
  return _filelengthi64(m_hFile);
}

CFile::ERRCODE CFile::SetSize(FILESIZE iSize)
{
  ASSERT(IsValid());
  int iRes = _chsize_s(m_hFile, iSize);
  return (iRes < 0) ? (ERRCODE) errno : 0; 
}

CFile::FILESIZE CFile::GetPos() const
{
  ASSERT(IsValid());
  return _telli64(m_hFile);
}

CFile::ERRCODE CFile::SetPos(FILESIZE iPos)
{
  ASSERT(IsValid());
  FILESIZE iRes = _lseeki64(m_hFile, iPos, SEEK_SET);
  return (iRes < 0) ? (ERRCODE) errno : 0; 
}

CFile::ERRCODE CFile::Read(void *pBuf, int iBytes, int *pProcessedBytes)
{
  ASSERT(IsValid());
  int iRead;
  if (!pProcessedBytes)
    pProcessedBytes = &iRead;
  *pProcessedBytes = _read(m_hFile, pBuf, iBytes);
  return (*pProcessedBytes == iBytes) ? 0 : (errno ? (ERRCODE) errno : EIO); 
}

CFile::ERRCODE CFile::Write(void const *pBuf, int iBytes, int *pProcessedBytes)
{
  ASSERT(IsValid());
  int iWritten;
  if (!pProcessedBytes)
    pProcessedBytes = &iWritten;
  *pProcessedBytes = _write(m_hFile, pBuf, iBytes);
  return (*pProcessedBytes == iBytes) ? 0 : (ERRCODE) errno; 
}

// CFileSystem ----------------------------------------------------------------

CFileSystem::CFileIter::~CFileIter()
{
  _findclose(m_hFind);
}

CFileSystem::CFileIter::operator bool() const
{
  return m_hFind != -1 && m_FindData.name[0];
}

CFileSystem::CFileIter &CFileSystem::CFileIter::operator ++()
{
  int iRes = _findnext64(m_hFind, &m_FindData);
  if (iRes)
    m_FindData.name[0] = 0;
  return *this;
}

CFileSystem::CFileIter &CFileSystem::CFileIter::operator =(const CStrBase &sPath)
{
  _findclose(m_hFind);
  m_hFind = _findfirst64(sPath, &m_FindData);
  return *this;
}

CStrPart CFileSystem::CFileIter::GetName() const
{
  return CStrPart(m_FindData.name);
}

CFileBase::FILESIZE CFileSystem::CFileIter::GetSize() const
{
  return m_FindData.size;
}

UINT CFileSystem::CFileIter::GetAttributes() const
{
  return m_FindData.attrib;
}

IMPRTTI(CFileSystem, CObject)

CFileSystem *CFileSystem::s_pFileSystem = 0;

CFileSystem::CFileSystem()
{
  s_pFileSystem = this;
  GetCurrentDirectory(m_sRootPath);
  m_sRootPath += CStrPart("/");
}

CFileSystem::~CFileSystem()
{
}

CFileBase *CFileSystem::OpenFile(const CStrBase &sFile, unsigned int uiFlags)
{
  CStr sPath = ResolvePath(sFile);
  CFileBase *pFile = new CFile(sPath, uiFlags);
  if (!pFile->IsValid()) {
    delete pFile;
    pFile = 0;
  }
  return pFile;
}

CFileBase::ERRCODE CFileSystem::DeleteFile(const CStrBase &sFile)
{
  CStr sPath = ResolvePath(sFile);
  int iRes = _unlink(sPath);
  return (iRes < 0) ? (CFileBase::ERRCODE) errno : 0; 
}

CFileBase::ERRCODE CFileSystem::GetCurrentDirectory(CStrBase &sDir)
{
  char *pPath = _getcwd(0, 0);
  if (!pPath) {
    sDir = CStrPart();
    return (CFileBase::ERRCODE) errno;
  }
  sDir.Assign(CStr(pPath));
  sDir.Assign(ResolvePath(sDir));
  free(pPath);
  return 0;
}

CFileBase::ERRCODE CFileSystem::SetCurrentDirectory(const CStrBase &sDir)
{
  CStr sPath = ResolvePath(sDir);
  int iRes = _chdir(sPath);
  return (iRes < 0) ? (CFileBase::ERRCODE) errno : 0; 
}

CFileBase::ERRCODE CFileSystem::CreateDirectory(const CStrBase &sDir)
{
  CStr sPath = ResolvePath(sDir);
  int iRes = _mkdir(sPath);
  return (iRes < 0) ? (CFileBase::ERRCODE) errno : 0; 
}

CFileBase::ERRCODE CFileSystem::DeleteDirectory(const CStrBase &sDir)
{
  CStr sPath = ResolvePath(sDir);
  int iRes = _rmdir(sPath);
  return (iRes < 0) ? (CFileBase::ERRCODE) errno : 0; 
}

CStr CFileSystem::ResolvePath(const CStrBase &sFile)
{
  CStr sPath, sTempDir, sRes;
  int i;
  bool bSkipSlash = false;
  for (i = 0; i < sFile.Length(); i++) {
    char ch = sFile[i];
    if (ch == '\\')
      ch = '/';
    if (ch != '/' || !bSkipSlash)
      sPath += CStrPart(&ch, 1);
    bSkipSlash = (ch == '/');
  }
  CStrPart sDrive, sDir, sName, sExt;
  ParsePath(sPath, &sDrive, &sDir, &sName, &sExt);

  CStrPart sRootDrive, sRootDir, sRootName, sRootExt;

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
      iPos = sTempDir.Find(CStrPart("/.."));
      if (iPos < 0)
        break;
      if (!iPos) 
        sTempDir = CStrPart("/") + sTempDir.SubStr(3, sTempDir.Length());
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

CStr CFileSystem::GetCurrentRoot()
{
  return m_sRootPath;
}

void CFileSystem::SetCurrentRoot(const CStrBase &sPath)
{
  m_sRootPath = CStrPart();
  CStr sPath1 = sPath;
  sPath1 += CStrPart("/");
  m_sRootPath = ResolvePath(sPath1);
}

bool CFileSystem::ParsePath(const CStrBase &sFile, CStrPart *pDrive, CStrPart *pPath, CStrPart *pName, CStrPart *pExtension)
{
  CStrPart sDrive, sPath, sName, sExt;
  int i;
  if (!pDrive)
    pDrive = &sDrive;
  else
    *pDrive = CStrPart();
  if (!pPath)
    pPath = &sPath;
  else
    *pPath = CStrPart();
  if (!pName)
    pName = &sName;
  else
    *pName = CStrPart();
  if (!pExtension)
    pExtension = &sExt;
  else
    *pExtension = CStrPart();
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
