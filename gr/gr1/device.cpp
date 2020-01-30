#include "device.h"
#include "output_pass.h"
#include "execution_queue.h"

NAMESPACE_BEGIN(gr1)

std::shared_ptr<Resource> Device::CreateResource(rttr::type resourceType)
{
	return _resourceFactory.CreateInstanceShared<Resource, Device&>(resourceType, *this);
}

NAMESPACE_END(gr1)

