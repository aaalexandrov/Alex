#ifndef __LINFILE_H
#define __LINFILE_H

#ifdef LINUX

#include "File.h"
#include <dirent.h>

class CLinFile: public CFile {
  DEFRTTI(CLinFile, CFile, true)
public:
  CStrAny    m_sFileName;
  int        m_hFile;

  explicit CLinFile();
  virtual ~CLinFile();

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

class CLinFileSystem: public CFileSystem {
  DEFRTTI(CLinFileSystem, CFileSystem, true)
public:
  class CLinFileIter: public CFileIter {
    DEFRTTI(CLinFileIter, CFileSystem::CFileIter, true)
  public:
    DIR             *m_pDir;
    struct dirent64 *m_pEntry;
    CStrAny          m_sPattern;

    CLinFileIter() { m_pDir = 0; m_pEntry = 0; }
    virtual ~CLinFileIter();

    virtual operator bool() const;
    virtual CFileIter &operator ++();
    virtual CFileIter &operator =(CStrAny const &sPath);

    virtual CStrAny GetName() const;
    virtual CFile::FILESIZE GetSize() const;
    virtual UINT GetAttributes() const;
  };
public:

  CLinFileSystem()          {}
  virtual ~CLinFileSystem() {}

  virtual CFile::ERRCODE DeleteFile(CStrAny const &sFile);

  virtual CFile::ERRCODE GetCurrentDirectory(CStrAny &sDir);
  virtual CFile::ERRCODE SetCurrentDirectory(CStrAny const &sDir);
  virtual CFile::ERRCODE CreateDirectory(CStrAny const &sDir);
  virtual CFile::ERRCODE DeleteDirectory(CStrAny const &sDir);
  virtual CRTTI const *GetFileRTTI() const     { return CLinFile::GetRTTI_s(); }
  virtual CRTTI const *GetFileIterRTTI() const { return CLinFileSystem::CLinFileIter::GetRTTI_s(); }
};

#endif

#endif
