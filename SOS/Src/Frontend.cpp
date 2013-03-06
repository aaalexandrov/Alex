#include "stdafx.h"
#include "Frontend.h"
#include "Interpreter.h"
#include "Compiler.h"

CInterpreter g_kInterpreter;

int ProcessInput()
{
  char chBuf[1024];

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
        err = g_kInterpreter.Execute(CValue(kChain.m_kCompiler.m_pCode), arrParams);
				if (err == IERR_OK) {
          for (int i = 0; i < arrParams.m_iCount; ++i) {
						CValue const &kVal = arrParams[i];
						CStrAny sRes = kVal.GetStr(true);
						fprintf(stdout, "<< %s\n", sRes.m_pBuf);
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