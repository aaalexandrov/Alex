#include "core.h"

namespace alang {

std::unique_ptr<ModuleDef> CreateCore()
{
	auto core = std::make_unique<ModuleDef>("Core");

	core->RegisterDef(std::make_unique<TypeDef>("TypeDef", sizeof(TypeDef), alignof(TypeDef)));
	core->RegisterDef(std::make_unique<TypeDef>("None", 0, 0));

	core->RegisterDef(std::make_unique<TypeDef>("Storage", 0, 0));

	core->RegisterDef(std::make_unique<TypeDef>("I8", sizeof(int8_t), alignof(int8_t)));
	core->RegisterDef(std::make_unique<TypeDef>("I16", sizeof(int16_t), alignof(int16_t)));
	core->RegisterDef(std::make_unique<TypeDef>("I32", sizeof(int32_t), alignof(int32_t)));
	core->RegisterDef(std::make_unique<TypeDef>("I64", sizeof(int64_t), alignof(int64_t)));

	core->RegisterDef(std::make_unique<TypeDef>("U8", sizeof(uint8_t), alignof(uint8_t)));
	core->RegisterDef(std::make_unique<TypeDef>("U16", sizeof(uint16_t), alignof(uint16_t)));
	core->RegisterDef(std::make_unique<TypeDef>("U32", sizeof(uint32_t), alignof(uint32_t)));
	core->RegisterDef(std::make_unique<TypeDef>("U64", sizeof(uint64_t), alignof(uint64_t)));

	core->RegisterDef(std::make_unique<TypeDef>("F32", sizeof(float), alignof(float)));
	core->RegisterDef(std::make_unique<TypeDef>("F64", sizeof(double), alignof(double)));

	core->RegisterDef(std::make_unique<TypeDef>("Bool", sizeof(bool), alignof(bool)));

	core->RegisterDef(std::make_unique<TypeDef>("Address", sizeof(uintptr_t), alignof(uintptr_t)));
	core->RegisterDef(std::make_unique<TypeDef>("Ref", sizeof(void*), alignof(void*)));
	
	core->RegisterDef(std::make_unique<TypeDef>("Array", 0, 0));

	core->RegisterDef(std::make_unique<TypeDef>("Func", sizeof(void*), alignof(void*)));

	return core;
}

}