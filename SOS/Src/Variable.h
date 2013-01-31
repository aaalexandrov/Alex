#ifndef __VARIABLE_H
#define __VARIABLE_H

#include "Hash.h"
#include "Str.h"

class CValueTable;
class CFragment;
class CValue {
public:
  enum EValueType {
    VT_NONE,
    VT_FLOAT,
    VT_STRING,
    VT_TABLE,
    VT_REF,
		VT_FRAGMENT,
  };

  typedef CHashKV<CStrHeader const *, CValue, CStrHeader, CStrHeader> THash;

public:
  union {
    float             m_fValue;
    CStrHeader const *m_pStrValue;
    CValueTable      *m_pTableValue;
    CValue           *m_pReference; 
		CFragment        *m_pFragment;
    void             *m_Value;
  };
  BYTE m_btType;

  CValue()                             { SetNone(); }
  CValue(float fValue)                 { Set(fValue); }
  CValue(CStrHeader const *pStrHeader) { Set(pStrHeader); }
  CValue(CValueTable *pTable)          { Set(pTable); }
  CValue(CValue *pReference)           { Set(pReference); }
	CValue(CFragment *pFragment)         { Set(pFragment); }
  CValue(CValue const &kValue)         { Set(kValue); }

  ~CValue() { ReleaseValue(); }

  inline void SetNone();
  inline void Set(float fValue);
  inline void Set(CStrHeader const *pStrHeader);
  inline void Set(CValueTable *pTable);
  inline void Set(CValue *pReference);
	inline void Set(CFragment *pFragment);
	inline void Set(CValue const &kValue);

  inline void AcquireValue();
  inline void ReleaseValue();
	inline void ClearValue() { ReleaseValue(); SetNone(); }

  inline bool operator ==(CValue const &kValue) const { return m_btType == kValue.m_btType && m_Value == kValue.m_Value; }
  inline bool operator !=(CValue const &kValue) const { return !(*this == kValue); }

  inline CValue &operator =(CValue const &kValue);

  inline float        GetFloat() const     { return m_btType == VT_FLOAT ? m_fValue : 0; }
  inline CStrAny      GetStr() const;
  inline CValueTable *GetTable() const     { return m_btType == VT_TABLE ? m_pTableValue : 0; }
  inline CValue      *GetReference() const { return m_btType == VT_REF ? m_pReference : 0; }
	inline CFragment   *GetFragment() const  { return m_btType == VT_FRAGMENT ? m_pFragment : 0; }
};

class CValueTable;

class CEnvRegistry {
public:
  typedef CHash<CValueTable *> THashValues;

public:
  THashValues m_hashValues;
  THashValues m_hashEnvironments;
  BYTE        m_btLastMark;
  int         m_iMarked;

  CEnvRegistry(): m_btLastMark(0), m_iMarked(0) {}
  ~CEnvRegistry() { Clear(); }

  static CEnvRegistry &Get() { static CEnvRegistry kRegistry; return kRegistry; }

  void Add(CValueTable *pValueTable)    { m_hashValues.Add(pValueTable); }
  void Remove(CValueTable *pValueTable) { m_hashValues.RemoveValue(pValueTable); }

  void Clear();

  void MarkAndSweep();
};

class CValueTable {
public:
  CValue::THash m_Hash;
  BYTE          m_btMark;

  CValueTable(): m_btMark(0) { CEnvRegistry::Get().Add(this); }
  ~CValueTable()             { CEnvRegistry::Get().Remove(this); }

  inline void Mark(BYTE btMark);
};

class CExecution;
class CInstruction;
class CFragment {
	DEFREFCOUNT
public:
  CArray<CInstruction> m_arrCode;
  CArray<CStrHeader *> m_arrInputs;

  void Append(CInstruction const &kInstr) { m_arrCode.Append(kInstr); }

  EInterpretError Execute(CExecution *pExecution);
	CInstruction *GetFirstInstruction() const { return m_arrCode.m_iCount ? m_arrCode.m_pArray : 0; }
  CInstruction *GetNextInstruction(CInstruction *pInstruction) const;

  void Dump();
};

// Implementation -------------------------------------------------------------

// CValue ---------------------------------------------------------------------
void CValue::AcquireValue()
{
	switch (m_btType) {
		case VT_STRING:
			m_pStrValue->Acquire();
			break;
		case VT_FRAGMENT:
			m_pFragment->Acquire();
			break;
	}
}

void CValue::ReleaseValue()
{
	switch (m_btType) {
		case VT_STRING:
			m_pStrValue->Release();
			break;
		case VT_FRAGMENT:
			m_pFragment->Release();
			break;
	}
}

void CValue::SetNone() 
{ 
  m_btType = VT_NONE; 
  m_Value = 0; 
}

void CValue::Set(float fValue)
{ 
  m_btType = VT_FLOAT; 
  m_fValue = fValue; 
}

void CValue::Set(CStrHeader const *pStrHeader) 
{ 
  m_btType = VT_STRING; 
  m_pStrValue = pStrHeader; 
  pStrHeader->Acquire(); 
}

void CValue::Set(CValueTable *pTable)    
{ 
  m_btType = VT_TABLE; 
  m_pTableValue = pTable; 
}

void CValue::Set(CValue *pReference)    
{ 
  m_btType = VT_REF; 
  m_pReference = pReference; 
}

void CValue::Set(CFragment *pFragment)    
{ 
  m_btType = VT_FRAGMENT; 
  m_pFragment = pFragment; 
	AcquireValue();
}

void CValue::Set(CValue const &kValue)
{
  m_btType = kValue.m_btType;
  m_Value = kValue.m_Value;
  AcquireValue();
}

CValue &CValue::operator =(CValue const &kValue) 
{ 
	CValue oldVal;
	// Manually copy the value to a temp without acquiring, it will be autoreleased when the temp goes out of scope
	oldVal.m_btType = m_btType;
	oldVal.m_Value = m_Value;
	// Set acquires the new value
	Set(kValue);
  return *this;
	// Original value gets autoreleased
}

CStrAny CValue::GetStr() const
{
  char chBuf[64];
  CStrAny s;
  switch (m_btType) {
    case VT_NONE:
      s = CStrAny(ST_CONST, "<null>");
      break;
    case VT_FLOAT:
      sprintf(chBuf, "%g", m_fValue);
      s = CStrAny(ST_CONST, chBuf);
      break;
    case VT_STRING:
      s = CStrAny(m_pStrValue);
      break;
    case VT_TABLE:
      sprintf(chBuf, "<Table:%x>", m_pTableValue);
      s = CStrAny(ST_CONST, chBuf);
      break;
    case VT_REF:
      sprintf(chBuf, "<Ref:%x>", m_pReference);
      s = CStrAny(ST_CONST, chBuf);
      break;
		case VT_FRAGMENT:
			sprintf(chBuf, "<Fragment:%x>", m_pFragment);
			s = CStrAny(ST_CONST, chBuf);
			break;
  }
  return s;
}

// CValueTable ----------------------------------------------------------------

void CValueTable::Mark(BYTE btMark)
{
  if (m_btMark == btMark)
    return;
  m_btMark = btMark;
  CEnvRegistry::Get().m_iMarked++;
  CValue::THash::TIter it;
  for (it = m_Hash; it; ++it) {
    if ((*it).m_Val.m_btType == CValue::VT_TABLE)
      (*it).m_Val.m_pTableValue->Mark(btMark);
  }
}

#endif