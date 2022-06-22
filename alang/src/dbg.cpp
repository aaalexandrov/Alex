#include "dbg.h"

#include <array>
#include <stdio.h>
#if defined(_WIN32)
#include <windows.h>
#endif

namespace dbg {

void AssertFailed(char const *message, char const *file, unsigned line)
{
	std::array<char, 1024> msg;
	snprintf(msg.data(), msg.size(), "%s(%u): Assert failed: %s", file, line, message);
	LogLine(std::cerr, msg.data());

#if defined(_WIN32)
	int res = MessageBoxA(nullptr, msg.data(), "Assert failed", MB_ABORTRETRYIGNORE | MB_DEFBUTTON2 | MB_ICONEXCLAMATION | MB_TASKMODAL | MB_SETFOREGROUND);
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

}