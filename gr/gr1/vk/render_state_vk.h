#pragma once 

#include "../render_state.h"
#include "vk.h"

NAMESPACE_BEGIN(gr1)

class RenderStateVk : public RenderState {
	RTTR_ENABLE(RenderState)
public:
	RenderStateVk(Device &device) : RenderState(device) {}

	void Init() override;

protected:

	void FillViewportState(vk::PipelineViewportStateCreateInfo &viewportState, std::vector<vk::Viewport> &viewports, std::vector<vk::Rect2D> &scissors);
	void FillRasterizationState(vk::PipelineRasterizationStateCreateInfo &rasterState);
	void FillMultisampleState(vk::PipelineMultisampleStateCreateInfo &multisampleState, std::vector<uint32_t> &sampleMask);
	void FillDepthStencilState(vk::PipelineDepthStencilStateCreateInfo &depthState);
	void FillBlendState(vk::PipelineColorBlendStateCreateInfo &blendState, std::vector<vk::PipelineColorBlendAttachmentState> &attachmentBlends);
	void FillDynamicState(vk::PipelineDynamicStateCreateInfo &dynamicState, std::vector<vk::DynamicState> &dynamicStates);

	void FillStencilOpState(StencilFuncState const &src, vk::StencilOpState &dst);

	static util::ValueRemapper<RenderState::FrontFaceMode, vk::FrontFace> s_frontFace2Vk;
	static util::ValueRemapper<RenderState::CullMask, vk::CullModeFlagBits> s_cullMask2Vk;
	static util::ValueRemapper<RenderState::CompareFunc, vk::CompareOp> s_compareFunc2Vk;
	static util::ValueRemapper<RenderState::StencilFunc, vk::StencilOp> s_stencilFunc2Vk;
	static util::ValueRemapper<RenderState::BlendFunc, vk::BlendOp> s_blendFunc2Vk;
	static util::ValueRemapper<RenderState::BlendFactor, vk::BlendFactor> s_blendFactor2Vk;
	static util::ValueRemapper<RenderState::ColorComponentMask, vk::ColorComponentFlagBits, true> s_colorComponentMask2Vk;
};

NAMESPACE_END(gr1)