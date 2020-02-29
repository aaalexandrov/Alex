#include "render_pipeline_vk.h"
#include "device_vk.h"
#include "shader_vk.h"
#include "render_pass_vk.h"
#include "render_pipeline_transition_pass_vk.h"
#include "rttr/registration.h"

NAMESPACE_BEGIN(gr1)

RTTR_REGISTRATION
{
	using namespace rttr;
	registration::class_<RenderPipelineVk>("RenderPipelineVk")(metadata(RttrDiscriminator::Tag, type::get<DeviceVk>()))
		.constructor<Device&>()(policy::ctor::as_raw_ptr);
}

void RenderPipelineVk::SetResourceState(ResourceState state)
{
	ASSERT((bool)_pipeline == (_state != ResourceState::ShaderRead));
	if (state == ResourceState::Invalidated) {
		_pipeline.reset();
	}
	RenderPipeline::SetResourceState(state);
}

std::shared_ptr<ResourceStateTransitionPass> RenderPipelineVk::CreateTransitionPass(ResourceState srcState, ResourceState dstState)
{
	auto transition = std::make_shared<RenderPipelineTransitionPassVk>(*_device);
	transition->Init(AsSharedPtr<Resource>(), srcState, dstState);
	return std::move(transition);
}

void RenderPipelineVk::UpdatePipeline()
{
	ASSERT(!_pipeline);
	DeviceVk *deviceVk = GetDevice<DeviceVk>();

	PipelineInfo pipeInfo;
	for (int i = 0; i < _shaders.size(); ++i) {
		pipeInfo._shaders[i] = static_cast<ShaderVk*>(_shaders[i].get());
	}

	pipeInfo._vertexBufferLayouts = _vertexBufferLayouts;

	std::shared_ptr<RenderPass> renderPass = _renderPass.lock();
	RenderPassVk *renderPassVk = static_cast<RenderPassVk*>(renderPass.get());
	pipeInfo._renderStateData = &_renderState->GetStateData();
	pipeInfo._primitiveKind = _primitiveKind;
	pipeInfo._renderPass = renderPassVk;
	pipeInfo._subpass = renderPassVk->GetSubpass();

	_pipeline = deviceVk->_pipelineStore.GetPipeline(pipeInfo);
}

NAMESPACE_END(gr1)

