#include "device.h"
#include "output_pass.h"

NAMESPACE_BEGIN(gr1)

std::shared_ptr<Resource> Device::CreateResource(rttr::type resourceType)
{
	return _resourceFactory.CreateInstanceShared<Resource, Device&>(resourceType, *this);
}


void ExecutionQueue::EnqueuePass(std::shared_ptr<OutputPass> pass)
{
	_passes.emplace_back(pass);
}

void ExecutionQueue::ExecutePasses()
{
	for (auto &pass : _passes) {
		pass->Prepare();
	}

	for (auto &pass : _passes) {
		pass->Execute();
	}

	WaitExecutionFinished();

	_passes.clear();
}


NAMESPACE_END(gr1)

