#ifndef __INTERPRETER_H
#define __INTERPRETER_H

#include "Execution.h"
#include "Library.h"

class CInterpreter {
public:
  CValueRegistry m_kValueRegistry;
  CSmartPtr<CValueTable> m_pGlobalEnvironment;
  CExecution m_kExecution;

  CInterpreter() : m_kExecution(this), m_pGlobalEnvironment(new CValueTable(&m_kValueRegistry)) { CFunctionLibrary::Init(*this); }
  ~CInterpreter() {}

  void GetTableValue(CValueTable const &kTable, CValue const &kKey, CValue &kValue) { CInstruction::GetTableValue(kTable, kKey, kValue); }
  void SetTableValue(CValueTable &kTable, CValue const &kKey, CValue const &kValue) { CInstruction::SetTableValue(kTable, kKey, kValue); }

  void GetGlobal(CValue const &kKey, CValue &kValue)       { CInstruction::GetTableValue(*m_pGlobalEnvironment, kKey, kValue); }
  void SetGlobal(CValue const &kKey, CValue const &kValue) { CInstruction::SetTableValue(*m_pGlobalEnvironment, kKey, kValue); }

  EInterpretError Execute(CValue const &kCode, CArray<CValue> &arrParams);
};


#endif