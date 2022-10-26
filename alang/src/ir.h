#pragma once

#include "types.h"

namespace alang {

enum class OpCode : uint8_t {
	Invalid,

	CallExternal,

	Call,
	Copy,
	JumpIfNonZero,
};

struct Instruction {
	OpCode _op = OpCode::Invalid;
	uint8_t _sizeDst : 3;
	uint8_t _indirectDst : 1;
	uint8_t _sizeArg1 : 3;
	uint8_t _indirectArg1 : 1;
	uint8_t _sizeArg2 : 3;
	uint8_t _indirectArg2 : 1;

	uintptr_t _dst = 0;
	uintptr_t _arg1 = 0;
	uintptr_t _arg2 = 0;
};

}