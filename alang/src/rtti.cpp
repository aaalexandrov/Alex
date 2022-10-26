#include "rtti.h"

namespace rtti {

struct Test {
	Test() 
	{
		auto ti = Get<int>();

		FuncImpl<int, int, char> fi;
		fi._func = [](int a, char b)->int { return a * b; };
		auto sig = fi.GetSignatureTypes();

		auto ss = fi.Invoke<int>(4, (char)5);
		auto sss = fi.Invoke<int>(42);

		FuncImpl<void> fv;
		int z = 0;
		fv._func = [&]() { ++z; };
		fv.Invoke<void>(4);
		fv.Invoke<void>();

		auto tt = Get<std::pair<int, char>>();
	}
} Tst;

}