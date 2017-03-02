#ifndef __FILE_H
#define __FILE_H

#include "../Base/Base.h"
#include "../Base/Str.h"

class CFile: public CObject {
  DEFRTTI(CFile, CObject, false)
public:
  typedef long long FILESIZE;
  typedef int ERRCODE;

  enum EOpenFlags {
    FOF_READ = 1,
    FOF_WRITE = 2,
    FOF_CREATE = 4,
    FOF_TRUNCATE = 8,
    FOF_APPEND = 16,
    FOF_TEXT = 32,
  };

  enum EFileAttributes {
    FA_READ = 1,
    FA_WRITE = 2,
    FA_EXECUTE = 4,
    FA_DIRECTORY = 8,
  };

  virtual ~CFile() {}

  virtual void     Init(CStrAny const &sName, unsigned int uiFlags) = 0;

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

class CFileSystem: public CObject {
  DEFRTTI(CFileSystem, CObject, false)
public:
  class CFileIter: public CObject {
    DEFRTTI(CFileSystem::CFileIter, CObject, false)
  public:
    virtual ~CFileIter() {}

    virtual operator bool() const = 0;
    virtual CFileIter &operator ++() = 0;
    virtual CFileIter &operator =(CStrAny const &sPath) = 0;

    virtual CStrAny GetName() const = 0;
    virtual CFile::FILESIZE GetSize() const = 0;
    virtual UINT GetAttributes() const = 0;
  };
public:
  static CFileSystem *s_pFileSystem;
  static inline CFileSystem *Get() { return s_pFileSystem; }

  CStrAny m_sRootPath;

  static void Create();
  static void Destroy();

  CFileSystem();
  virtual ~CFileSystem();

  virtual void Init();

  virtual CFile *OpenFile(CStrAny const &sFile, unsigned int uiFlags);

  virtual CFile::ERRCODE DeleteFile(CStrAny const &sFile) = 0;

  virtual CFile::ERRCODE GetCurrentDirectory(CStrAny &sDir) = 0;
  virtual CFile::ERRCODE SetCurrentDirectory(CStrAny const &sDir) = 0;
  virtual CFile::ERRCODE CreateDirectory(CStrAny const &sDir) = 0;
  virtual CFile::ERRCODE DeleteDirectory(CStrAny const &sDir) = 0;
  virtual CRTTI const *GetFileRTTI() const = 0;
  virtual CRTTI const *GetFileIterRTTI() const = 0;

  virtual CFileIter *GetFileIter(CStrAny const &sPath);

  virtual CStrAny ResolvePath(CStrAny const &sFile);
  virtual CStrAny GetCurrentRoot();
  virtual void SetCurrentRoot(CStrAny const &sPath);

  static bool ParsePath(CStrAny const &sFile, CStrAny *pDrive, CStrAny *pPath, CStrAny *pName, CStrAny *pExtension);
};

// Implementation -------------------------------------------------------------

template <class T>
CFile::ERRCODE CFile::Write(T const &t)
{
  ERRCODE err = Write(&t, sizeof(t));
  return err;
}

template <class T>
CFile::ERRCODE CFile::Read(T &t)
{
  ERRCODE err = Read(&t, sizeof(t));
  return err;
}

template <class T>
CFile::ERRCODE CFile::WriteArray(T const &t)
{
  ERRCODE err = Write(&t.m_iCount, sizeof(t.m_iCount));
  if (err || !t.m_iCount)
    return err;
  err = Write(t.m_pArray, t.m_iCount * sizeof(*t.m_pArray));
  return err;
}

template <class T>
CFile::ERRCODE CFile::ReadArray(T &t)
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
