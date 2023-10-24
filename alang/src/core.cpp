#include "core.h"
#include "types.h"

namespace alang {

static std::unique_ptr<Module> CreateCore()
{
	auto core = std::make_unique<Module>("Core");

	core->RegisterDefinition(std::make_unique<TypeDesc>("TypeDesc", sizeof(TypeDesc), alignof(TypeDesc)));

	core->RegisterDefinition(std::make_unique<TypeDesc>("Storage", 0, 0));

	core->RegisterDefinition(std::make_unique<TypeDesc>("I8", sizeof(int8_t), alignof(int8_t)));
	core->RegisterDefinition(std::make_unique<TypeDesc>("I16", sizeof(int16_t), alignof(int16_t)));
	core->RegisterDefinition(std::make_unique<TypeDesc>("I32", sizeof(int32_t), alignof(int32_t)));
	core->RegisterDefinition(std::make_unique<TypeDesc>("I64", sizeof(int64_t), alignof(int64_t)));

	core->RegisterDefinition(std::make_unique<TypeDesc>("U8", sizeof(uint8_t), alignof(uint8_t)));
	core->RegisterDefinition(std::make_unique<TypeDesc>("U16", sizeof(uint16_t), alignof(uint16_t)));
	core->RegisterDefinition(std::make_unique<TypeDesc>("U32", sizeof(uint32_t), alignof(uint32_t)));
	core->RegisterDefinition(std::make_unique<TypeDesc>("U64", sizeof(uint64_t), alignof(uint64_t)));

	core->RegisterDefinition(std::make_unique<TypeDesc>("F32", sizeof(float), alignof(float)));
	core->RegisterDefinition(std::make_unique<TypeDesc>("F64", sizeof(double), alignof(double)));

	core->RegisterDefinition(std::make_unique<TypeDesc>("Bool", sizeof(bool), alignof(bool)));

	core->RegisterDefinition(std::make_unique<TypeDesc>("Address", sizeof(uintptr_t), alignof(uintptr_t)));
	core->RegisterDefinition(std::make_unique<TypeDesc>("Ref", sizeof(void*), alignof(void*)));
	
	core->RegisterDefinition(std::make_unique<TypeDesc>("Array", 0, 0));

	core->RegisterDefinition(std::make_unique<TypeDesc>("Func", sizeof(void*), alignof(void*)));

	return core;
}

std::unique_ptr<Module> CoreModule = CreateCore();

}