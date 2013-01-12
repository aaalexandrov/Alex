#include "stdafx.h"
#include "Frontend.h"
#include "Grammar.h"
#include "Interpreter.h"
#include "Compile.h"

int ProcessInput(CInterpreter &kInterp)
{
  char chBuf[1024];
  CExecution kExecution;

	while (1) {
		fputs("> ", stdout);
		if (!fgets(chBuf, ARRSIZE(chBuf), stdin))
			return 0;
		CExpression kExpr;
		EInterpretError err = IERR_OK;
    CStrAny sInput(ST_WHOLE, chBuf);
//		err = kExpr.Init(sInput);
    if (err == IERR_OK) {
//		  err = kExpr.Execute(kInterp.m_kStack, kInterp.m_Vars);
      if (err == IERR_OK) {
        CCompileChain kChain;
        CArray<CValue> arrParams;
        err = kChain.Compile(sInput);
        if (err == IERR_OK) {
          kChain.m_kCompiler.m_pCode->Dump();
          err = kExecution.Execute(kChain.m_kCompiler.m_pCode, arrParams);
          if (kExecution.m_kStack.m_iCount != 1)
            err = IERR_UNKNOWN;
          if (err == IERR_OK) {
			      CStrAny sRes = kExecution.m_kStack.Last().GetStr();
			      fprintf(stdout, "<< %s\n", sRes.m_pBuf);
          }
        }
      }
    }
/*		if (!kInterp.m_kStack.m_iCount)
			err = IERR_UNKNOWN;
		if (err == IERR_OK) {
			CStrAny sRes;
			kInterp.m_kStack.Head()->GetVar()->GetStr(sRes);
			fprintf(stdout, "< %s\n", sRes.m_pBuf);
		} else {
			fprintf(stdout, "ERROR %d\n", err);
		}
		kInterp.m_kStack.DeleteAll();
*/
	}

	return 0;
}