#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <memory>
#include "dbg.h"
#include "rtti.h"

namespace alang {

using String = std::string;

struct PosInFile {
	String _fileName;
	uint32_t _line = ~0u;
	uint32_t _posOnLine = ~0u;

	String ToString() const;
};

struct Error {
	String _error;
	PosInFile _location;

	Error() = default;
	Error(String err, PosInFile const &loc) : _error(err), _location(loc) {}

	operator bool() const { return !_error.empty(); }

	String ToString() const;
};


struct Module;
struct ParseNode;
struct Definition: public rtti::Any {
	String _name;
	Module *_parentModule = nullptr;
	ParseNode const *_node = nullptr;

	Definition(String name);
	Definition(ParseNode const *node);
	virtual ~Definition() {}

	String GetQualifiedName() const;
	void GetQualifiedName(std::vector<String> &name) const;

	rtti::TypeInfo const *GetTypeInfo() const override;

	virtual Error Analyze() = 0;
};

}

namespace rtti {
template <> TypeInfo const *Get<alang::Definition>();
}