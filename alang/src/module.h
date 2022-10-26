#pragma once

#include "common.h"
#include "types.h"
#include "var.h"
#include "func.h"
#include <unordered_map>

namespace alang {

struct Module : public Definition {

	std::unordered_map<String, std::unique_ptr<Definition>> _definitions;

	Module(String name);

	void RegisterDefinition(std::unique_ptr<Definition> &&def);
};

}