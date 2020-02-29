#include "render_pipeline_transition_pass_vk.h"
#include "device_vk.h"
#include "rttr/registration.h"

NAMESPACE_BEGIN(gr1)

RTTR_REGISTRATION
{
	using namespace rttr;
	registration::class_<RenderPipelineTransitionPassVk>("RenderPipelineTransitionPassVk")(metadata(RttrDiscriminator::Tag, type::get<DeviceVk>()))
		.constructor<Device&>()(policy::ctor::as_raw_ptr);
}

RenderPipelineTransitionPassVk::RenderPipelineTransitionPassVk(Device &device)
	: ResourceStateTransitionPass(device)
{
}

void RenderPipelineTransitionPassVk::GetDependencies(DependencyType dependencyType, DependencyFunc addDependencyFunc)
{
	ResourceStateTransitionPass::GetDependencies(dependencyType, addDependencyFunc);
	RenderPipeline *pipeline = GetResource<RenderPipeline>();
	if (dependencyType == DependencyType::Input) {
		for (auto &shader : pipeline->GetShaders()) {
			if (shader) {
				addDependencyFunc(shader.get(), ResourceState::ShaderRead);
			}
		}
		addDependencyFunc(pipeline->GetRenderState().get(), ResourceState::ShaderRead);
	}
}


void RenderPipelineTransitionPassVk::Prepare()
{
	ASSERT(_srcState == ResourceState::Initial || _srcState == ResourceState::Invalidated);
	ASSERT(_dstState == ResourceState::ShaderRead);

	RenderPipelineVk *pipelineVk = GetResource<RenderPipelineVk>();
	pipelineVk->UpdatePipeline();
}

void RenderPipelineTransitionPassVk::Submit(PassDependencyTracker &dependencies)
{
}


NAMESPACE_END(gr1)

