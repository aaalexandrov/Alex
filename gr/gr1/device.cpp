#include "device.h"
#include "output_pass.h"

NAMESPACE_BEGIN(gr1)

std::shared_ptr<Resource> Device::CreateResource(rttr::type resourceType)
{
	return _resourceFactory.CreateInstanceShared<Resource, Device&>(resourceType, *this);
}

void ExecutionQueue::EnqueuePass(std::shared_ptr<OutputPass> const &pass)
{
	_passes.emplace_back(pass);
}

NAMESPACE_END(gr1)

