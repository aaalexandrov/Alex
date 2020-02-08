#include "device.h"
#include "output_pass.h"
#include "execution_queue.h"

NAMESPACE_BEGIN(gr1)

std::shared_ptr<ResourceBase> Device::CreateResource(rttr::type resourceType)
{
	return _resourceFactory.CreateInstanceShared<ResourceBase, Device&>(resourceType, *this);
}

NAMESPACE_END(gr1)

