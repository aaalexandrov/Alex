#include "stdafx.h"
#include "LinFile.h"

#ifdef LINUX

#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <fnmatch.h>

// CLinFile ----------------------------------------------------------
CRTTIRegisterer<CLinFile> g_RegLinFile;

CLinFile::CLinFile()
{
  m_hFile = 0;
}

void CLinFile::Init(CStrAny const &sName, unsigned int uiFlags)
{
  int iOpenFlags = 0;
  mode_t mMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH; // Owner, group and everyone have R/W permissions
  m_sFileName = sName;
  if (uiFlags & FOF_READ) {
    if (uiFlags & FOF_WRITE) {
      iOpenFlags |= O_RDWR;
    } else
      iOpenFlags |= O_RDONLY;
  } else
    if (uiFlags & FOF_WRITE) {
      iOpenFlags |= O_WRONLY;
    }
  if (uiFlags & FOF_CREATE)
    iOpenFlags |= O_CREAT;
  if (uiFlags & FOF_APPEND)
    iOpenFlags |= O_APPEND;
  if (uiFlags & FOF_TRUNCATE)
    iOpenFlags |= O_TRUNC;
  // FOF_TEXT is ignored
  m_hFile = open(sName.m_pBuf, iOpenFlags, mMode);
}

CLinFile::~CLinFile()
{
  if (m_hFile > 0)
    close(m_hFile);
}

CFile::FILESIZE CLinFile::GetSize() const
{
  ASSERT(IsValid());
  struct stat64 kStat;
  int iRes = fstat64(m_hFile, &kStat);
  if (iRes < 0)
    return -1;
  return kStat.st_size;
}

CFile::ERRCODE CLinFile::SetSize(FILESIZE iSize)
{
  ASSERT(IsValid());
  int iRes = ftruncate64(m_hFile, iSize);
  return (iRes < 0) ? (ERRCODE) errno : 0;
}

CFile::FILESIZE CLinFile::GetPos() const
{
  ASSERT(IsValid());
  FILESIZE iCurPos = lseek64(m_hFile, 0, SEEK_CUR);
  return iCurPos;
}

CFile::ERRCODE CLinFile::SetPos(FILESIZE iPos)
{
  ASSERT(IsValid());
  FILESIZE iRes = lseek64(m_hFile, iPos, SEEK_SET);
  return (iRes < 0) ? (ERRCODE) errno : 0;
}

CFile::ERRCODE CLinFile::Read(void *pBuf, int iBytes, int *pProcessedBytes)
{
  ASSERT(IsValid());
  int iRead;
  if (!pProcessedBytes)
    pProcessedBytes = &iRead;
  *pProcessedBytes = read(m_hFile, pBuf, iBytes);
  return (*pProcessedBytes == iBytes) ? 0 : (errno ? (ERRCODE) errno : EIO);
}

CFile::ERRCODE CLinFile::Write(void const *pBuf, int iBytes, int *pProcessedBytes)
{
  ASSERT(IsValid());
  int iWritten;
  if (!pProcessedBytes)
    pProcessedBytes = &iWritten;
  *pProcessedBytes = write(m_hFile, pBuf, iBytes);
  return (*pProcessedBytes == iBytes) ? 0 : (ERRCODE) errno;
}

// CLinFileSystem -------------------------------------------------------------

CLinFileSystem::CLinFileIter::~CLinFileIter()
{
  closedir(m_pDir);
}

CLinFileSystem::CLinFileIter::operator bool() const
{
  return m_pDir && m_pEntry;
}

CFileSystem::CFileIter &CLinFileSystem::CLinFileIter::operator ++()
{
  if (m_pDir) {
    while (1) {
      m_pEntry = readdir64(m_pDir);
      if (!m_pEntry)
        break;
      if (!fnmatch(m_sPattern.m_pBuf, m_pEntry->d_name, FNM_NOESCAPE | FNM_PATHNAME))
        break;
    }
  }
  return *this;
}

CFileSystem::CFileIter &CLinFileSystem::CLinFileIter::operator =(CStrAny const &sPath)
{
  closedir(m_pDir);
  m_pDir = 0;
  m_pEntry = 0;
  CStrAny sDrive, sDir, sName, sExt;
  if (!CFileSystem::ParsePath(sPath, &sDrive, &sDir, &sName, &sExt))
    return *this;
  CStrAny sDest = sDrive + sDir;
  if (!sDest.ZeroTerminated())
    sDest.MakeUnique();
  m_pDir = opendir(sDest.m_pBuf);
  m_sPattern = sName + sExt;
  return operator ++();
}

CStrAny CLinFileSystem::CLinFileIter::GetName() const
{
  if (!m_pEntry)
    return CStrAny();
  return CStrAny(ST_WHOLE, m_pEntry->d_name);
}

CFile::FILESIZE CLinFileSystem::CLinFileIter::GetSize() const
{
  if (!m_pEntry)
    return -1;
  struct stat64 kStat;
  int iRes = stat64(m_pEntry->d_name, &kStat);
  if (iRes < 0)
    return -1;
  return kStat.st_size;
}

UINT CLinFileSystem::CLinFileIter::GetAttributes() const
{
  if (!m_pEntry)
    return -1;
  struct stat64 kStat;
  int iRes = stat64(m_pEntry->d_name, &kStat);
  if (iRes < 0)
    return -1;
  UINT uiAttr = 0;
  if (kStat.st_mode & S_IRUSR)
    uiAttr |= CFile::FA_READ;
  if (kStat.st_mode & S_IWUSR)
    uiAttr |= CFile::FA_WRITE;
  if (kStat.st_mode & S_IXUSR)
    uiAttr |= CFile::FA_EXECUTE;
  if (S_ISDIR(kStat.st_mode))
    uiAttr |= CFile::FA_DIRECTORY;
  return uiAttr;
}

CRTTIRegisterer<CLinFileSystem> g_RegLinFileSystem;
CRTTIRegisterer<CLinFileSystem::CLinFileIter> g_RegLinFileSystemLinFileIter;

CFile::ERRCODE CLinFileSystem::DeleteFile(CStrAny const &sFile)
{
  CStrAny sPath = ResolvePath(sFile);
  int iRes = unlink(sPath.m_pBuf);
  return (iRes < 0) ? (CFile::ERRCODE) errno : 0;
}

CFile::ERRCODE CLinFileSystem::GetCurrentDirectory(CStrAny &sDir)
{
  char chPath[PATH_MAX];
  if (!getcwd(chPath, ARRSIZE(chPath))) {
    sDir.Clear();
    return (CFile::ERRCODE) errno;
  }
  sDir = ResolvePath(CStrAny(ST_WHOLE, chPath));
  return 0;
}

CFile::ERRCODE CLinFileSystem::SetCurrentDirectory(CStrAny const &sDir)
{
  CStrAny sPath = ResolvePath(sDir);
  int iRes = chdir(sPath.m_pBuf);
  return (iRes < 0) ? (CFile::ERRCODE) errno : 0;
}

CFile::ERRCODE CLinFileSystem::CreateDirectory(CStrAny const &sDir)
{
  CStrAny sPath = ResolvePath(sDir);
  mode_t mMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH; // Owner, group and everyone have R/W permissions
  int iRes = mkdir(sPath.m_pBuf, mMode);
  return (iRes < 0) ? (CFile::ERRCODE) errno : 0;
}

CFile::ERRCODE CLinFileSystem::DeleteDirectory(CStrAny const &sDir)
{
  CStrAny sPath = ResolvePath(sDir);
  int iRes = rmdir(sPath.m_pBuf);
  return (iRes < 0) ? (CFile::ERRCODE) errno : 0;
}

#endif
