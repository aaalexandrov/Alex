#include "stdafx.h"
#include "WinFile.h"

#ifdef WINDOWS

#include <io.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <direct.h>
#include <ctype.h>
#include <stdio.h>

// CWinFile ----------------------------------------------------------
CRTTIRegisterer<CWinFile> g_RegWinFile;

CWinFile::CWinFile()
{
  m_hFile = 0;
}

void CWinFile::Init(CStrAny const &sName, unsigned int uiFlags)
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
  m_hFile = _open(sName.m_pBuf, iOpenFlags, iCreate);
}

CWinFile::~CWinFile()
{
  if (m_hFile > 0)
    _close(m_hFile);
}

CFile::FILESIZE CWinFile::GetSize() const
{
  ASSERT(IsValid());
  return _filelengthi64(m_hFile);
}

CFile::ERRCODE CWinFile::SetSize(FILESIZE iSize)
{
  ASSERT(IsValid());
  int iRes = _chsize(m_hFile, (long) iSize);
  return (iRes < 0) ? (ERRCODE) errno : 0;
}

CFile::FILESIZE CWinFile::GetPos() const
{
  ASSERT(IsValid());
  return _telli64(m_hFile);
}

CFile::ERRCODE CWinFile::SetPos(FILESIZE iPos)
{
  ASSERT(IsValid());
  FILESIZE iRes = _lseeki64(m_hFile, iPos, SEEK_SET);
  return (iRes < 0) ? (ERRCODE) errno : 0;
}

CFile::ERRCODE CWinFile::Read(void *pBuf, int iBytes, int *pProcessedBytes)
{
  ASSERT(IsValid());
  int iRead;
  if (!pProcessedBytes)
    pProcessedBytes = &iRead;
  *pProcessedBytes = _read(m_hFile, pBuf, iBytes);
  return (*pProcessedBytes == iBytes) ? 0 : (errno ? (ERRCODE) errno : EIO);
}

CFile::ERRCODE CWinFile::Write(void const *pBuf, int iBytes, int *pProcessedBytes)
{
  ASSERT(IsValid());
  int iWritten;
  if (!pProcessedBytes)
    pProcessedBytes = &iWritten;
  *pProcessedBytes = _write(m_hFile, pBuf, iBytes);
  return (*pProcessedBytes == iBytes) ? 0 : (ERRCODE) errno;
}

// CWinFileSystem -------------------------------------------------------------

CWinFileSystem::CWinFileIter::~CWinFileIter()
{
  _findclose(m_hFind);
}

CWinFileSystem::CWinFileIter::operator bool() const
{
  return m_hFind != -1 && m_FindData.name[0];
}

CFileSystem::CFileIter &CWinFileSystem::CWinFileIter::operator ++()
{
  int iRes = _findnexti64(m_hFind, &m_FindData);
  if (iRes)
    m_FindData.name[0] = 0;
  return *this;
}

CFileSystem::CFileIter &CWinFileSystem::CWinFileIter::operator =(CStrAny const &sPath)
{
  _findclose(m_hFind);
  m_hFind = _findfirsti64(sPath.m_pBuf, &m_FindData);
  return *this;
}

CStrAny CWinFileSystem::CWinFileIter::GetName() const
{
  return CStrAny(ST_WHOLE, m_FindData.name);
}

CFile::FILESIZE CWinFileSystem::CWinFileIter::GetSize() const
{
  return m_FindData.size;
}

UINT CWinFileSystem::CWinFileIter::GetAttributes() const
{
  return m_FindData.attrib;
}

CRTTIRegisterer<CWinFileSystem> g_RegWinFileSystem;
CRTTIRegisterer<CWinFileSystem::CWinFileIter> g_RegWinFileSystemWinFileIter;

CFile::ERRCODE CWinFileSystem::DeleteFile(CStrAny const &sFile)
{
  CStrAny sPath = ResolvePath(sFile);
  int iRes = _unlink(sPath.m_pBuf);
  return (iRes < 0) ? (CFile::ERRCODE) errno : 0;
}

CFile::ERRCODE CWinFileSystem::GetCurrentDirectory(CStrAny &sDir)
{
  char *pPath = _getcwd(0, 0);
  if (!pPath) {
    sDir.Clear();
    return (CFile::ERRCODE) errno;
  }
  sDir = ResolvePath(CStrAny(ST_WHOLE, pPath));
  free(pPath);
  return 0;
}

CFile::ERRCODE CWinFileSystem::SetCurrentDirectory(CStrAny const &sDir)
{
  CStrAny sPath = ResolvePath(sDir);
  int iRes = _chdir(sPath.m_pBuf);
  return (iRes < 0) ? (CFile::ERRCODE) errno : 0;
}

CFile::ERRCODE CWinFileSystem::CreateDirectory(CStrAny const &sDir)
{
  CStrAny sPath = ResolvePath(sDir);
  int iRes = _mkdir(sPath.m_pBuf);
  return (iRes < 0) ? (CFile::ERRCODE) errno : 0;
}

CFile::ERRCODE CWinFileSystem::DeleteDirectory(CStrAny const &sDir)
{
  CStrAny sPath = ResolvePath(sDir);
  int iRes = _rmdir(sPath.m_pBuf);
  return (iRes < 0) ? (CFile::ERRCODE) errno : 0;
}

#endif
