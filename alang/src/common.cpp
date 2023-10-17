#include "common.h"
#include "module.h"
#include "parse.h"

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

}

