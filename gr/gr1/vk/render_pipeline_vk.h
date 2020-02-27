#pragma once

#include "../render_pipeline.h"
#include "vk.h"

NAMESPACE_BEGIN(gr1)

struct PipelineVk;
class RenderPipelineVk : public RenderPipeline {
	RTTR_ENABLE(RenderPipeline)
public:
	RenderPipelineVk(Device &device) : RenderPipeline(device) {}

	void Init() override;
	std::shared_ptr<ResourceStateTransitionPass> CreateTransitionPass(ResourceState srcState, ResourceState dstState) override;

public:
	std::shared_ptr<PipelineVk> _pipeline;
};

NAMESPACE_END(gr1)