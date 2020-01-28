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

std::shared_ptr<OutputPass> ExecutionQueue::CreateTransitionPass(Resource *resource, ResourceState srcState, ResourceState dstState)
{
	rttr::type passType = resource->GetStateTransitionPassType();
	rttr::variant passVar = passType.create({ *_device });
	auto pass = passVar.get_value<std::shared_ptr<ResourceStateTransitionPass>>();
	pass->Init(*resource, srcState, dstState);
	return pass;
}

NAMESPACE_END(gr1)

