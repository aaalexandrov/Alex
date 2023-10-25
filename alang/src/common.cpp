#include "common.h"
#include "module.h"
#include "parse.h"
#include <sstream>
#include <iostream>

namespace alang {

String PosInFile::ToString() const
{
	return _fileName + ":" + std::to_string(_line) + ":" + std::to_string(_posOnLine);
}

String Error::ToString() const
{
	if (_error.empty())
		return "No error";
	return _location.ToString() + " - " + _error;
}

std::vector<String> SplitString(String str, char delimiter)
{
	std::vector<String> split;
	std::istringstream f(str);
	String s;
	while (getline(f, s, delimiter)) {
		split.push_back(s);
	}
	return split;
}

}

