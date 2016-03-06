#ifndef __VARIABLE_H
#define __VARIABLE_H

#include "Hash.h"
#include "Str.h"

class CValueTable;
class CFragment;
class CExecution;
class CValue {
public:
  enum EValueType {
    VT_NONE,
		VT_BOOL,
    VT_FLOAT,
    VT_STRING,
    VT_TABLE,
		VT_FRAGMENT,
    VT_NATIVE_FUNC,

    VT_LAST
  };

  typedef CHashKV<CValue, CValue, CValue, CValue> THash;
  typedef EInterpretError (FnNative)(CExecution &kExecution, CArray<CValue> &arrParamResults);

public:
  union {
		bool              m_bValue;
    float             m_fValue;
    CStrHeader const *m_pStrValue;
    CValueTable      *m_pTableValue;
		CFragment        *m_pFragment;
    FnNative         *m_pNativeFunc;
    void             *m_Value;
  };
  uint8_t m_btType;

  explicit CValue()                             { SetNone(); }
	explicit CValue(bool bValue)                  { Set(bValue); }
  explicit CValue(float fValue)                 { Set(fValue); }
  explicit CValue(CStrHeader const *pStrHeader) { Set(pStrHeader); }
  explicit CValue(CValueTable *pTable)          { Set(pTable); }
	explicit CValue(CFragment *pFragment)         { Set(pFragment); }
  explicit CValue(FnNative *pNativeFunc)        { Set(pNativeFunc); }
  CValue(CValue const &kValue)                  { Set(kValue); }

  ~CValue() { ReleaseValue(); }

  inline void SetNone();
	inline void Set(bool bValue);
  inline void Set(float fValue);
  inline void Set(CStrHeader const *pStrHeader);
  inline void Set(CValueTable *pTable);
	inline void Set(CFragment *pFragment);
  inline void Set(FnNative *pNativeFunc);
	inline void Set(CValue const &kValue);

  inline void AcquireValue();
  inline void ReleaseValue();
	inline void ClearValue() { ReleaseValue(); SetNone(); }
  inline void DeleteValue();

  inline bool operator ==(CValue const &kValue) const { return m_btType == kValue.m_btType && m_Value == kValue.m_Value; }
  inline bool operator !=(CValue const &kValue) const { return !(*this == kValue); }

  inline CValue &operator =(CValue const &kValue);

	inline bool         GetBool() const       { return !(m_btType == VT_NONE || m_btType == VT_BOOL && !m_bValue); }
	inline float        GetFloat() const      { return m_btType == VT_FLOAT ? m_fValue : 0; }
  inline CStrAny      GetStr(bool bDecorate) const;
  inline CValueTable *GetTable() const      { return m_btType == VT_TABLE ? m_pTableValue : 0; }
	inline CFragment   *GetFragment() const   { return m_btType == VT_FRAGMENT ? m_pFragment : 0; }
  inline FnNative    *GetNativeFunc() const { return m_btType == VT_NATIVE_FUNC ? m_pNativeFunc : 0; }

	inline size_t GetHash() const { return (size_t) m_btType ^ (size_t) m_Value; }

	static inline size_t Hash(CValue const &kVal)                   { return kVal.GetHash(); }
  static inline bool Eq(CValue const &kVal0, CValue const &kVal1) { return kVal0 == kVal1; }

  static CValue2String::TValueString s_arrVT2Str[VT_LAST];
  static CValue2String s_VT2Str;
};

class CValueTable;
class CInterpreter;

class CValueRegistry {
public:
  typedef CHash<CValue, CValue, CValue, CValue> THashValues;

public:
  THashValues *m_pUnprocessed, *m_pProcessed; // White, black set for tri-color garbage collecting
  CList<CValue> m_lstProcessing; // Grey set

  CValueRegistry();
  ~CValueRegistry();

  void Add(CValue const &kValue);
  void Remove(CValue const &kValue);

  void DeleteValues(THashValues &hashValues);

  void MoveToProcessing(CValue const &kValue);
  void Process(CList<CValue>::TNode *pNode);
  void CollectGarbage(CInterpreter *pInterpreter);
};

class CValueTable {
public:
  CValue::THash m_Hash;

  CValueTable(CValueRegistry *pRegistry) { pRegistry->Add(CValue(this)); }
  ~CValueTable() {}
};

class CExecution;
class CInstruction;
class CFragment {
public:
  CArray<CInstruction> m_arrCode;
	CArray<CValue> m_arrConst;
	short m_nLocalCount, m_nParamCount;

	CFragment(CValueRegistry *pRegistry): m_nLocalCount(0), m_nParamCount(0) { pRegistry->Add(CValue(this)); }
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
	}
}

void CValue::ReleaseValue()
{
	switch (m_btType) {
		case VT_STRING:
			m_pStrValue->Release();
			break;
	}
}

void CValue::DeleteValue()
{
	switch (m_btType) {
		case VT_FRAGMENT:
			DEL(m_pFragment);
			break;
		case VT_TABLE:
			DEL(m_pTableValue);
			break;
    default:
      ASSERT(0);
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
}

void CValue::Set(CFragment *pFragment)
{
  m_btType = VT_FRAGMENT;
  m_pFragment = pFragment;
}

void CValue::Set(FnNative *pNativeFunc)
{
  m_btType = VT_NATIVE_FUNC;
  m_pNativeFunc = pNativeFunc;
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

CStrAny CValue::GetStr(bool bDecorate) const
{
  char chBuf[64];
  CStrAny s;
  switch (m_btType) {
    case VT_NONE:
      s = CStrAny(ST_CONST, "nil");
      if (bDecorate)
        s = CStrAny(ST_CONST, '<') + s + CStrAny(ST_CONST, '>');
      break;
		case VT_BOOL:
			s = CStrAny(ST_CONST, m_bValue ? "true" : "false");
      if (bDecorate)
        s = CStrAny(ST_CONST, '<') + s + CStrAny(ST_CONST, '>');
			break;
    case VT_FLOAT:
      s = CStrAny(ST_CONST, m_fValue);
      break;
    case VT_STRING:
      s = CStrAny(m_pStrValue);
      if (bDecorate)
        s = CStrAny(ST_CONST, '"') + s + CStrAny(ST_CONST, '"');
      break;
    case VT_TABLE:
      sprintf(chBuf, "<Table:%p>", m_pTableValue);
      s = CStrAny(ST_CONST, chBuf);
      break;
		case VT_FRAGMENT:
			sprintf(chBuf, "<Fragment:%p>", m_pFragment);
			s = CStrAny(ST_CONST, chBuf);
			break;
		case VT_NATIVE_FUNC:
			sprintf(chBuf, "<NativeFunc:%p>", m_pNativeFunc);
			s = CStrAny(ST_CONST, chBuf);
			break;
  }
  return s;
}

#endif
