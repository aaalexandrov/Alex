#include "dbg.h"
#include <stdio.h>

NAMESPACE_BEGIN(util)

void AssertFailed(char const *message, char const *file, unsigned line)
{
	char msg[1024];
	sprintf_s(msg, ArraySize(msg), "%s(%u): Assert failed: %s", file, line, message);
	LogLine(std::cerr, msg);

#if defined(_WIN32)
	int res = MessageBoxA(nullptr, msg, "Assert failed", MB_ABORTRETRYIGNORE | MB_DEFBUTTON2 | MB_ICONEXCLAMATION | MB_TASKMODAL | MB_SETFOREGROUND);
	switch (res) {
		case IDABORT:
			std::abort();
			break;
		case IDRETRY:
			DebugBreak();
			break;
		case IDIGNORE:
			break;
	}
#endif
}

NAMESPACE_END(util)

