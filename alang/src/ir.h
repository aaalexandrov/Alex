#pragma once

#include "types.h"

namespace alang {

enum class OpCode : uint16_t {
	Invalid,

};

struct Instruction {
	OpCode _op = OpCode::Invalid;

	uintptr_t _dst = 0;
	uintptr_t _arg1 = 0;
	uintptr_t _arg2 = 0;
};

}