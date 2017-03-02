#ifndef __WINFILE_H
#define __WINFILE_H

#ifdef WINDOWS

#include "../File.h"
#include <io.h>

class CWinFile: public CFile {
  DEFRTTI(CWinFile, CFile, true)
public:
  CStrAny    m_sFileName;
  int        m_hFile;

  explicit CWinFile();
  virtual ~CWinFile();

  virtual void Init(CStrAny const &sName, unsigned int uiFlags);

  virtual bool     IsValid() const     { return m_hFile > 0; }
  virtual CStrAny  GetFullName() const { return m_sFileName; }

  virtual FILESIZE GetSize() const;
  virtual ERRCODE  SetSize(FILESIZE iSize);
  virtual FILESIZE GetPos() const;
  virtual ERRCODE  SetPos(FILESIZE iPos);
  virtual ERRCODE  Read(void *pBuf, int iBytes, int *pProcessedBytes = 0);
  virtual ERRCODE  Write(void const *pBuf, int iBytes, int *pProcessedBytes = 0);
};

class CWinFileSystem: public CFileSystem {
  DEFRTTI(CWinFileSystem, CFileSystem, true)
public:
  class CWinFileIter: public CFileIter {
    DEFRTTI(CWinFileIter, CFileSystem::CFileIter, true)
  public:
    intptr_t       m_hFind;
    _finddatai64_t m_FindData;

    CWinFileIter() { m_hFind = -1; }
    virtual ~CWinFileIter();

    virtual operator bool() const;
    virtual CFileIter &operator ++();
    virtual CFileIter &operator =(CStrAny const &sPath);

    virtual CStrAny GetName() const;
    virtual CFile::FILESIZE GetSize() const;
    virtual UINT GetAttributes() const;
  };
public:

  CWinFileSystem()          {}
  virtual ~CWinFileSystem() {}

  virtual CFile::ERRCODE DeleteFile(CStrAny const &sFile);

  virtual CFile::ERRCODE GetCurrentDirectory(CStrAny &sDir);
  virtual CFile::ERRCODE SetCurrentDirectory(CStrAny const &sDir);
  virtual CFile::ERRCODE CreateDirectory(CStrAny const &sDir);
  virtual CFile::ERRCODE DeleteDirectory(CStrAny const &sDir);
  virtual CRTTI const *GetFileRTTI() const     { return CWinFile::GetRTTI_s(); }
  virtual CRTTI const *GetFileIterRTTI() const { return CWinFileSystem::CWinFileIter::GetRTTI_s(); }
};

#endif

#endif
