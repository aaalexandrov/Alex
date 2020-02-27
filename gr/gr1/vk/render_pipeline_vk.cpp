#include "render_pipeline_vk.h"
#include "device_vk.h"
#include "render_pipeline_transition_pass_vk.h"
#include "rttr/registration.h"

NAMESPACE_BEGIN(gr1)

RTTR_REGISTRATION
{
	using namespace rttr;
	registration::class_<RenderPipelineVk>("RenderPipelineVk")(metadata(RttrDiscriminator::Tag, type::get<DeviceVk>()))
		.constructor<Device&>()(policy::ctor::as_raw_ptr);
}

void RenderPipelineVk::Init()
{
	RenderPipeline::Init();
}

std::shared_ptr<ResourceStateTransitionPass> RenderPipelineVk::CreateTransitionPass(ResourceState srcState, ResourceState dstState)
{
	auto transition = std::make_shared<RenderPipelineTransitionPassVk>(*_device);
	transition->Init(AsSharedPtr<Resource>(), srcState, dstState);
	return std::move(transition);
}


NAMESPACE_END(gr1)
