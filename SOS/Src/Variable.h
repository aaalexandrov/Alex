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
		VT_BOOL,
    VT_FLOAT,
    VT_STRING,
    VT_TABLE,
		VT_FRAGMENT,
  };

  typedef CHashKV<CValue, CValue, CValue, CValue> THash;

public:
  union {
		bool                     m_bValue;
    float                    m_fValue;
    CStrHeader const        *m_pStrValue;
    CValueTable             *m_pTableValue;
		CFragment               *m_pFragment;
    void                    *m_Value;
  };
  BYTE m_btType;

  explicit CValue()                             { SetNone(); }
	explicit CValue(bool bValue)                  { Set(bValue); }
  explicit CValue(float fValue)                 { Set(fValue); }
  explicit CValue(CStrHeader const *pStrHeader) { Set(pStrHeader); }
  explicit CValue(CValueTable *pTable)          { Set(pTable); }
	explicit CValue(CFragment *pFragment)         { Set(pFragment); }
  CValue(CValue const &kValue)                  { Set(kValue); }

  ~CValue() { ReleaseValue(); }

  inline void SetNone();
	inline void Set(bool bValue);
  inline void Set(float fValue);
  inline void Set(CStrHeader const *pStrHeader);
  inline void Set(CValueTable *pTable);
	inline void Set(CFragment *pFragment);
	inline void Set(CValue const &kValue);

  inline void AcquireValue();
  inline void ReleaseValue();
	inline void ClearValue() { ReleaseValue(); SetNone(); }

  inline bool operator ==(CValue const &kValue) const { return m_btType == kValue.m_btType && m_Value == kValue.m_Value; }
  inline bool operator !=(CValue const &kValue) const { return !(*this == kValue); }

  inline CValue &operator =(CValue const &kValue);

	inline bool         GetBool() const      { return m_btType == VT_BOOL ? m_bValue : false; }
	inline float        GetFloat() const     { return m_btType == VT_FLOAT ? m_fValue : 0; }
  inline CStrAny      GetStr() const;
  inline CValueTable *GetTable() const     { return m_btType == VT_TABLE ? m_pTableValue : 0; }
	inline CFragment   *GetFragment() const  { return m_btType == VT_FRAGMENT ? m_pFragment : 0; }

	inline size_t GetHash() const { return (size_t) m_btType ^ (size_t) m_Value; }

	static inline size_t Hash(CValue const &kVal)                   { return kVal.GetHash(); }
  static inline bool Eq(CValue const &kVal0, CValue const &kVal1) { return kVal0 == kVal1; }
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
	DEFREFCOUNT
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
	CArray<CValue> m_arrConst;
	short m_nLocalCount, m_nParamCount;

	CFragment(): m_nLocalCount(0), m_nParamCount(0) {}
	~CFragment() {}

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
		case VT_TABLE:
			m_pTableValue->Acquire();
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
		case VT_TABLE:
			m_pTableValue->Release();
			break;
	}
}

void CValue::SetNone() 
{ 
  m_btType = VT_NONE; 
  m_Value = 0; 
}

void CValue::Set(bool bValue)
{
	m_btType = VT_BOOL;
	m_bValue = bValue;
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
  m_pStrValue->Acquire(); 
}

void CValue::Set(CValueTable *pTable)    
{ 
  m_btType = VT_TABLE; 
  m_pTableValue = pTable; 
	m_pTableValue->Acquire();
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
		case VT_BOOL:
			s = CStrAny(ST_CONST, m_bValue ? "<true>" : "<false>");
			break;
    case VT_FLOAT:
      sprintf(chBuf, "%g", m_fValue);
      s = CStrAny(ST_CONST, chBuf);
      break;
    case VT_STRING:
      s = CStrAny(ST_CONST, '"') + CStrAny(m_pStrValue) + CStrAny(ST_CONST, '"');
      break;
    case VT_TABLE:
      sprintf(chBuf, "<Table:%x>", m_pTableValue);
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