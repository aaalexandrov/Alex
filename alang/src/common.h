#pragma once

#include <stdint.h>
#include <vector>
#include <string>
#include <memory>
#include "dbg.h"

namespace alang {

using String = std::string;

struct PosInFile {
	uint32_t _line = -1;
	uint32_t _posOnLine = -1;
};

struct Module;
struct Definition {
	String _name;
	Module *_parentModule = nullptr;

	Definition(String name) : _name(name) {}
};

}