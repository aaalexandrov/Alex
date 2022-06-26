#pragma once

#include "common.h"
#include "types.h"
#include <unordered_map>

namespace alang {

struct TypeDesc;
struct Module {
	String _name;
	Module *_parent = nullptr;

	std::unordered_map<String, std::unique_ptr<TypeDesc>> _types;

	Module(String name, Module *parent = nullptr);

	void RegisterType(std::unique_ptr<TypeDesc> &&type);
};

}