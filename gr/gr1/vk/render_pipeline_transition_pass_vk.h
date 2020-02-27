#pragma once

#include "output_pass_vk.h"
#include "render_pipeline_vk.h"

NAMESPACE_BEGIN(gr1)

class RenderPipelineTransitionPassVk : public ResourceStateTransitionPass, public PassVk {
	RTTR_ENABLE(ResourceStateTransitionPass, PassVk)
public:

	RenderPipelineTransitionPassVk(Device &device);

	void GetDependencies(DependencyType dependencyType, DependencyFunc addDependencyFunc) override;

	void Prepare() override;
	void Execute(PassDependencyTracker &dependencies) override;

	vk::PipelineStageFlags GetPassDstStages() override { return vk::PipelineStageFlagBits::eTopOfPipe; }
public:

};

NAMESPACE_END(gr1)