#include "core.h"

namespace alang {

static std::unique_ptr<Module> CreateCore()
{
	auto core = std::make_unique<Module>("Core", nullptr);

	core->RegisterDefinition(std::make_unique<TypeDesc>("TypeDesc", nullptr, sizeof(void*), alignof(void*)));

	core->RegisterDefinition(std::make_unique<TypeDesc>("I8", nullptr, sizeof(int8_t), alignof(int8_t)));
	core->RegisterDefinition(std::make_unique<TypeDesc>("I16", nullptr, sizeof(int16_t), alignof(int16_t)));
	core->RegisterDefinition(std::make_unique<TypeDesc>("I32", nullptr, sizeof(int32_t), alignof(int32_t)));
	core->RegisterDefinition(std::make_unique<TypeDesc>("I64", nullptr, sizeof(int64_t), alignof(int64_t)));

	core->RegisterDefinition(std::make_unique<TypeDesc>("U8", nullptr, sizeof(uint8_t), alignof(uint8_t)));
	core->RegisterDefinition(std::make_unique<TypeDesc>("U16", nullptr, sizeof(uint16_t), alignof(uint16_t)));
	core->RegisterDefinition(std::make_unique<TypeDesc>("U32", nullptr, sizeof(uint32_t), alignof(uint32_t)));
	core->RegisterDefinition(std::make_unique<TypeDesc>("U64", nullptr, sizeof(uint64_t), alignof(uint64_t)));

	core->RegisterDefinition(std::make_unique<TypeDesc>("F32", nullptr, sizeof(float), alignof(float)));
	core->RegisterDefinition(std::make_unique<TypeDesc>("F64", nullptr, sizeof(double), alignof(double)));

	core->RegisterDefinition(std::make_unique<TypeDesc>("Bool", nullptr, sizeof(bool), alignof(bool)));

	core->RegisterDefinition(std::make_unique<TypeDesc>("Address", nullptr, sizeof(uintptr_t), alignof(uintptr_t)));
	core->RegisterDefinition(std::make_unique<TypeDesc>("Ref", nullptr, sizeof(void*), alignof(void*)));
	
	core->RegisterDefinition(std::make_unique<TypeDesc>("Array", nullptr, 0, 0));

	core->RegisterDefinition(std::make_unique<TypeDesc>("Const", nullptr, 0, 0));

	core->RegisterDefinition(std::make_unique<TypeDesc>("Func", nullptr, sizeof(void*), alignof(void*)));

	return core;
}

std::unique_ptr<Module> CoreModule = CreateCore();

}