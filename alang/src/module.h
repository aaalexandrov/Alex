#pragma once

#include "common.h"
#include <unordered_map>

namespace alang {

struct TypeDesc;
struct Module {
	String _name;
	Module *_parent;

	std::unordered_map<String, std::unique_ptr<TypeDesc>> _types;
};

}