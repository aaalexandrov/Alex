#ifndef __FILE_H
#define __FILE_H

#include "Base.h"
#include "Str.h"
#include <io.h>

class CFileBase: public CObject {
  DEFRTTI_NOCREATE
public:
  typedef __int64 FILESIZE;
  typedef int ERRCODE;

  enum EOpenFlags {
    FOF_READ = 1,
    FOF_WRITE = 2,
    FOF_CREATE = 4,
    FOF_TRUNCATE = 8,
    FOF_APPEND = 16,
    FOF_TEXT = 32,
  };

  virtual ~CFileBase() {}

  virtual bool     IsValid() const = 0;
  virtual CStrAny  GetFullName() const = 0;

  virtual FILESIZE GetSize() const = 0;
  virtual ERRCODE  SetSize(FILESIZE iSize) = 0;
  virtual FILESIZE GetPos() const = 0;
  virtual ERRCODE  SetPos(FILESIZE iPos) = 0;

  virtual ERRCODE  Read(void *pBuf, int iBytes, int *pProcessedBytes = 0) = 0;
  virtual ERRCODE  Write(void const *pBuf, int iBytes, int *pProcessedBytes = 0) = 0;

  template <class T>
  ERRCODE Write(T const &t);
  template <class T>
  ERRCODE Read(T &t);

  template <class T>
  ERRCODE Write(CArray<T> const &t) { return WriteArray(t); }
  template <class T>
  ERRCODE Read(CArray<T> &t) { return ReadArray(t); }

  template <class T>
  ERRCODE WriteArray(T const &t);
  template <class T>
  ERRCODE ReadArray(T &t);

  ERRCODE WriteBuf(void const *pBuf, int iBytes);
  ERRCODE ReadBuf(void *&pBuf, int &iBytes);
};

class CFile: public CFileBase {
  DEFRTTI_NOCREATE
public:
  CStrAny    m_sFileName;
  int        m_hFile;

  explicit CFile(CStrAny const &sName, unsigned int uiFlags);
  virtual ~CFile();

  virtual bool     IsValid() const     { return m_hFile > 0; }
  virtual CStrAny  GetFullName() const { return m_sFileName; }

  virtual FILESIZE GetSize() const;
  virtual ERRCODE  SetSize(FILESIZE iSize);
  virtual FILESIZE GetPos() const;
  virtual ERRCODE  SetPos(FILESIZE iPos);
  virtual ERRCODE  Read(void *pBuf, int iBytes, int *pProcessedBytes = 0);
  virtual ERRCODE  Write(void const *pBuf, int iBytes, int *pProcessedBytes = 0);
};

class CFileSystem: public CObject {
  DEFRTTI
public:
  class CFileIter {
  public:
    intptr_t       m_hFind;
    __finddata64_t m_FindData;

    CFileIter(CStrAny const &sPath) { m_hFind = -1; operator =(sPath); }
    ~CFileIter();

    operator bool() const;
    CFileIter &operator ++();
    CFileIter &operator =(CStrAny const &sPath);

    CStrAny GetName() const;
    CFileBase::FILESIZE GetSize() const;
    UINT GetAttributes() const;
  };
public:
  static CFileSystem *s_pFileSystem;
  static inline CFileSystem *Get() { return s_pFileSystem; }

  CStrAny m_sRootPath;

  CFileSystem();
  virtual ~CFileSystem();

  virtual CFileBase *OpenFile(CStrAny const &sFile, unsigned int uiFlags);

  virtual CFileBase::ERRCODE DeleteFile(CStrAny const &sFile);

  virtual CFileBase::ERRCODE GetCurrentDirectory(CStrAny &sDir);
  virtual CFileBase::ERRCODE SetCurrentDirectory(CStrAny const &sDir);
  virtual CFileBase::ERRCODE CreateDirectory(CStrAny const &sDir);
  virtual CFileBase::ERRCODE DeleteDirectory(CStrAny const &sDir);

  virtual CStrAny ResolvePath(CStrAny const &sFile);
  virtual CStrAny GetCurrentRoot();
  virtual void SetCurrentRoot(CStrAny const &sPath);

  static bool ParsePath(CStrAny const &sFile, CStrAny *pDrive, CStrAny *pPath, CStrAny *pName, CStrAny *pExtension);
};

// Implementation -------------------------------------------------------------

template <class T>
CFileBase::ERRCODE CFileBase::Write(T const &t)
{
  ERRCODE err = Write(&t, sizeof(t));
  return err;
}

template <class T>
CFileBase::ERRCODE CFileBase::Read(T &t)
{
  ERRCODE err = Read(&t, sizeof(t));
  return err;
}

template <class T>
CFileBase::ERRCODE CFileBase::WriteArray(T const &t)
{
  ERRCODE err = Write(&t.m_iCount, sizeof(t.m_iCount));
  if (err || !t.m_iCount)
    return err;
  err = Write(t.m_pArray, t.m_iCount * sizeof(*t.m_pArray));
  return err;
}

template <class T>
CFileBase::ERRCODE CFileBase::ReadArray(T &t)
{
  int iCount;
  ERRCODE err = Read(iCount);
  if (err)
    return err;
  t.SetCount(iCount);
  if (!iCount)
    return 0;
  err = Read(t.m_pArray, iCount * sizeof(*t.m_pArray));
  return err;
}

#endif