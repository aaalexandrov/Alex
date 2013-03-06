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
};

#endif