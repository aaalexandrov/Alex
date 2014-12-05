#ifndef __LIBRARY_H
#define __LIBRARY_H

#include "Variable.h"

class CInterpreter;
class CExecution;
class CFunctionLibrary {
public:
  static EInterpretError Init(CInterpreter &kInterpreter);

  static EInterpretError Print(CExecution &kExecution, CArray<CValue> &arrParams);
  static EInterpretError Dump(CExecution &kExecution, CArray<CValue> &arrParams);
  static EInterpretError Next(CExecution &kExecution, CArray<CValue> &arrParams);
  static EInterpretError Type(CExecution &kExecution, CArray<CValue> &arrParams);
  static EInterpretError ToString(CExecution &kExecution, CArray<CValue> &arrParams);
  static EInterpretError ToNumber(CExecution &kExecution, CArray<CValue> &arrParams);

	static EInterpretError Compile(CExecution &kExecution, CArray<CValue> &arrParams);
	static EInterpretError Eval(CExecution &kExecution, CArray<CValue> &arrParams);
};

#endif