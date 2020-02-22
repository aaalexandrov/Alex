#pragma once 

#include "../render_state.h"
#include "vk.h"

NAMESPACE_BEGIN(gr1)

class RenderStateVk : public RenderState {
	RTTR_ENABLE(RenderState)
public:
	RenderStateVk(Device &device) : RenderState(device) {}

	void Init() override;

public:

	static void FillViewportState(StateData const &data, vk::PipelineViewportStateCreateInfo &viewportState, std::vector<vk::Viewport> &viewports, std::vector<vk::Rect2D> &scissors);
	static void FillRasterizationState(StateData const &data, vk::PipelineRasterizationStateCreateInfo &rasterState);
	static void FillMultisampleState(StateData const &data, vk::PipelineMultisampleStateCreateInfo &multisampleState, std::vector<uint32_t> &sampleMask);
	static void FillDepthStencilState(StateData const &data, vk::PipelineDepthStencilStateCreateInfo &depthState);
	static void FillBlendState(StateData const &data, vk::PipelineColorBlendStateCreateInfo &blendState, std::vector<vk::PipelineColorBlendAttachmentState> &attachmentBlends);
	static void FillDynamicState(StateData const &data, vk::PipelineDynamicStateCreateInfo &dynamicState, std::vector<vk::DynamicState> &dynamicStates);

	static void FillViewports(StateData const &data, std::vector<vk::Viewport> &viewports);
	static void FillStencilOpState(StencilFuncState const &src, vk::StencilOpState &dst);

	static util::ValueRemapper<FrontFaceMode, vk::FrontFace> s_frontFace2Vk;
	static util::ValueRemapper<CullMask, vk::CullModeFlagBits> s_cullMask2Vk;
	static util::ValueRemapper<StencilFunc, vk::StencilOp> s_stencilFunc2Vk;
	static util::ValueRemapper<BlendFunc, vk::BlendOp> s_blendFunc2Vk;
	static util::ValueRemapper<BlendFactor, vk::BlendFactor> s_blendFactor2Vk;
	static util::ValueRemapper<ColorComponentMask, vk::ColorComponentFlagBits, true> s_colorComponentMask2Vk;
};

extern util::ValueRemapper<CompareFunc, vk::CompareOp> s_compareFunc2Vk;

NAMESPACE_END(gr1)