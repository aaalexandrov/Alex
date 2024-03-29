#pragma once

#include <cstdint>

namespace alang {

// Execution model
// 
// Code executes in functions
// Each function has 64k of local memory (stack/registers), and can address global memory offsets
// Local memory is addressed through 16-bit offset
// Global memory is addressed indirectly, through a 64-bit local value 
// (i.e. instructions contain the offset of the 64-bit local value that contains the offset in global memory)
// Function arguments are located at the start of the local memory
// Function calls expect all arguments to occupy contiguous local memory after the end of all live local memory values, 
// the offset of which is passed to the call instruction

// Code is contained in a separate memory region, and jumps/calls use offsets within this region

// Global memory contains constants and global variables at the start, and is dynamically growable to support runtime allocation

enum class Op : uint8_t {
	Move,
	Convert,

	Neg,
	Add,
	Sub,
	Mul,
	Div,

	Not,
	And,
	Or,
	Xor,

	Shift,

	Jump,
	Call,
	Return,

	Last
};

enum class OpAddressingMode : uint8_t {
	Immediate,
	Local,     
	Indirect, 

	Last
};

enum OperandType : uint8_t {
	U8,
	U16,
	U32,
	U64,

	S8,
	S16,
	S32,
	S64,

	F32,
	F64,

	Addr, // ??

	Last
};

enum OpDirection : uint8_t {
	ToLocal,
	ToModal,
};

struct OpCode {
	Op _op : 6;
	OpAddressingMode _addrMode : 2;
	OperandType _operType : 4;
	OpDirection _opDirection : 1;
	uint16_t _localOper;
	uint16_t _modalOper;
};

}