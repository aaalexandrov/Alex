#pragma once

#include "resource.h"

NAMESPACE_BEGIN(gr1)

enum class DependencyType {
	None = 0,
	Read = 1,
	Write = 2,
};

class OutputPass : public Resource {
	RTTR_ENABLE(Resource)
public:
	OutputPass(Device &device) : Resource(device) {}

	virtual void GetDependencies(DependencyType dependencyType, std::unordered_set<Resource*> &dependencies) = 0;

	virtual void Prepare() = 0;
	virtual void Execute() = 0;
};

NAMESPACE_END(gr1)

