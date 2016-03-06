#ifndef __INTERPRETER_H
#define __INTERPRETER_H

#include "Execution.h"
#include "Library.h"

class CInterpreter {
public:
  CValueRegistry m_kValueRegistry;
  CValueTable *m_pGlobalEnvironment;
  CExecution m_kExecution;
  size_t m_uiCollectionThreshold;

  CInterpreter();
  ~CInterpreter();

  void GetTableValue(CValueTable const &kTable, CValue const &kKey, CValue &kValue) { CInstruction::GetTableValue(kTable, kKey, kValue); }
  void SetTableValue(CValueTable &kTable, CValue const &kKey, CValue const &kValue) { CInstruction::SetTableValue(kTable, kKey, kValue); }

  void GetGlobal(CValue const &kKey, CValue &kValue)       { CInstruction::GetTableValue(*m_pGlobalEnvironment, kKey, kValue); }
  void SetGlobal(CValue const &kKey, CValue const &kValue) { CInstruction::SetTableValue(*m_pGlobalEnvironment, kKey, kValue); }

  inline size_t GetMemoryUsed() { return Instance<TSosAllocator>().m_uiSize; }
  void SetCollectionThreshold() { m_uiCollectionThreshold = Max<size_t>(GetMemoryUsed() * 2, 256); }

  EInterpretError Execute(CValue const &kCode, CArray<CValue> &arrParams);
  EInterpretError CollectGarbage();
  EInterpretError CheckForCollection();
};


#endif
