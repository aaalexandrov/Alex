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

  virtual bool      IsValid() const = 0;
  virtual CStrConst GetFullName() const = 0;

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
  CStrConst  m_sFileName;
  int        m_hFile;

  explicit CFile(const CStrBase &sName, unsigned int uiFlags);
  virtual ~CFile();

  virtual bool      IsValid() const     { return m_hFile > 0; }
  virtual CStrConst GetFullName() const { return m_sFileName; }

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

    CFileIter(const CStrBase &sPath) { m_hFind = -1; operator =(sPath); }
    ~CFileIter();

    operator bool() const;
    CFileIter &operator ++();
    CFileIter &operator =(const CStrBase &sPath);

    CStrPart GetName() const;
    CFileBase::FILESIZE GetSize() const;
    UINT GetAttributes() const;
  };
public:
  static CFileSystem *s_pFileSystem;
  static inline CFileSystem *Get() { return s_pFileSystem; }

  CStr m_sRootPath;

  CFileSystem();
  virtual ~CFileSystem();

  virtual CFileBase *OpenFile(const CStrBase &sFile, unsigned int uiFlags);

  virtual CFileBase::ERRCODE DeleteFile(const CStrBase &sFile);

  virtual CFileBase::ERRCODE GetCurrentDirectory(CStrBase &sDir);
  virtual CFileBase::ERRCODE SetCurrentDirectory(const CStrBase &sDir);
  virtual CFileBase::ERRCODE CreateDirectory(const CStrBase &sDir);
  virtual CFileBase::ERRCODE DeleteDirectory(const CStrBase &sDir);

  virtual CStr ResolvePath(const CStrBase &sFile);
  virtual CStr GetCurrentRoot();
  virtual void SetCurrentRoot(const CStrBase &sPath);

  static bool ParsePath(const CStrBase &sFile, CStrPart *pDrive, CStrPart *pPath, CStrPart *pName, CStrPart *pExtension);
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