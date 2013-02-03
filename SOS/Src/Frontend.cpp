#include "stdafx.h"
#include "Frontend.h"
#include "BNFGrammar.h"
#include "Compiler.h"

int ProcessInput()
{
  char chBuf[1024];
  CExecution kExecution;

	while (1) {
		fputs("> ", stdout);
		if (!fgets(chBuf, ARRSIZE(chBuf), stdin))
			return 0;
		EInterpretError err = IERR_OK;
    CStrAny sInput(ST_WHOLE, chBuf);
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
      } else
				if (err == IERR_COMPILE_FAILED) {
					kChain.m_kGrammar.Dump();
				}
    }
	}

	return 0;
}