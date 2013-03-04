#include "stdafx.h"
#include "Frontend.h"
#include "BNFGrammar.h"
#include "Compiler.h"

int ProcessInput()
{
  char chBuf[1024];
  CExecution kExecution;
	kExecution.m_pGlobalEnvironment = new CValueTable();

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
        err = kExecution.Execute(kChain.m_kCompiler.m_pCode, arrParams, kExecution.m_pGlobalEnvironment);
				if (err == IERR_OK) {
					for (int i = 0; i < kExecution.m_nReturnCount; ++i) {
						CValue const &kVal = kExecution.m_arrLocal[kExecution.m_nReturnBase + i];
						CStrAny sRes = kVal.GetStr(true);
						fprintf(stdout, "<< %s\n", sRes.m_pBuf);
						if (kVal.m_btType == CValue::VT_FRAGMENT)
							kVal.GetFragment()->Dump();
					}
				} else {
					fprintf(stdout, "<< %s\n", g_IERR2Str.GetStr(err).m_pBuf);
				}
      } else
				if (err == IERR_COMPILE_FAILED) {
					kChain.m_kGrammar.Dump();
				}
    }
	}

	return 0;
}