#include "render_state_vk.h"
#include "device_vk.h"
#include "../execution_queue.h"
#include "rttr/registration.h"

NAMESPACE_BEGIN(gr1)

RTTR_REGISTRATION
{
	using namespace rttr;
	registration::class_<RenderStateVk>("RenderStateVk")(metadata(RttrDiscriminator::Tag, type::get<DeviceVk>()))
		.constructor<Device&>()(policy::ctor::as_raw_ptr);
}


util::ValueRemapper<RenderState::FrontFaceMode, vk::FrontFace> RenderStateVk::s_frontFace2Vk { {
		{FrontFaceMode::CCW, vk::FrontFace::eCounterClockwise},
		{FrontFaceMode::CW, vk::FrontFace::eClockwise},
	} };

util::ValueRemapper<RenderState::CullMask, vk::CullModeFlagBits> RenderStateVk::s_cullMask2Vk{ {
		{CullMask::None, vk::CullModeFlagBits::eNone},
		{CullMask::Front, vk::CullModeFlagBits::eFront},
		{CullMask::Back, vk::CullModeFlagBits::eBack},
		{CullMask::FrontAndBack, vk::CullModeFlagBits::eFrontAndBack},
	} };

util::ValueRemapper<RenderState::CompareFunc, vk::CompareOp> RenderStateVk::s_compareFunc2Vk{ {
		{CompareFunc::Never, vk::CompareOp::eNever},
		{CompareFunc::Less, vk::CompareOp::eLess},
		{CompareFunc::Equal, vk::CompareOp::eEqual},
		{CompareFunc::LessOrEqual, vk::CompareOp::eLessOrEqual},
		{CompareFunc::Greater, vk::CompareOp::eGreater},
		{CompareFunc::NotEqual, vk::CompareOp::eNotEqual},
		{CompareFunc::GreaterOrEqual, vk::CompareOp::eGreaterOrEqual},
		{CompareFunc::Always, vk::CompareOp::eAlways},
	} };

util::ValueRemapper<RenderState::StencilFunc, vk::StencilOp> RenderStateVk::s_stencilFunc2Vk{ {
		{StencilFunc::Keep, vk::StencilOp::eKeep},
		{StencilFunc::Zero, vk::StencilOp::eZero},
		{StencilFunc::Replace, vk::StencilOp::eReplace},
		{StencilFunc::IncrementAndClamp, vk::StencilOp::eIncrementAndClamp},
		{StencilFunc::DecrementAndClamp, vk::StencilOp::eDecrementAndClamp},
		{StencilFunc::Invert, vk::StencilOp::eInvert},
		{StencilFunc::IncrementAndWrap, vk::StencilOp::eIncrementAndWrap},
		{StencilFunc::DecrementAndWrap, vk::StencilOp::eDecrementAndWrap},
	} };

util::ValueRemapper<RenderState::BlendFunc, vk::BlendOp> RenderStateVk::s_blendFunc2Vk{ {
		{BlendFunc::Add,             vk::BlendOp::eAdd,             },
		{BlendFunc::Subtract,        vk::BlendOp::eSubtract,        },
		{BlendFunc::ReverseSubtract, vk::BlendOp::eReverseSubtract, },
		{BlendFunc::Min,             vk::BlendOp::eMin,             },
		{BlendFunc::Max,             vk::BlendOp::eMax,             },
	} };

util::ValueRemapper<RenderState::BlendFactor, vk::BlendFactor> RenderStateVk::s_blendFactor2Vk{ {
		{BlendFactor::Zero,                     vk::BlendFactor::eZero,                  },
		{BlendFactor::One,                      vk::BlendFactor::eOne,                   },
		{BlendFactor::SrcColor,                 vk::BlendFactor::eSrcColor,              },
		{BlendFactor::OneMinusSrcColor,         vk::BlendFactor::eOneMinusSrcColor,      },
		{BlendFactor::DstColor,                 vk::BlendFactor::eDstColor,              },
		{BlendFactor::OneMinusDstColor,         vk::BlendFactor::eOneMinusDstColor,      },
		{BlendFactor::SrcAlpha,                 vk::BlendFactor::eSrcAlpha,              },
		{BlendFactor::OneMinusSrcAlpha,         vk::BlendFactor::eOneMinusSrcAlpha,      },
		{BlendFactor::DstAlpha,                 vk::BlendFactor::eDstAlpha,              },
		{BlendFactor::OneMinusDstAlpha,         vk::BlendFactor::eOneMinusDstAlpha,      },
		{BlendFactor::ConstantColor,            vk::BlendFactor::eConstantColor,         },
		{BlendFactor::OneMinusConstantColor,    vk::BlendFactor::eOneMinusConstantColor, },
		{BlendFactor::ConstantAlpha,            vk::BlendFactor::eConstantAlpha,         },
		{BlendFactor::OneMinusConstantAlpha,    vk::BlendFactor::eOneMinusConstantAlpha, },
		{BlendFactor::SrcAlphaSaturate,         vk::BlendFactor::eSrcAlphaSaturate,      },
		{BlendFactor::Src1Color,                vk::BlendFactor::eSrc1Color,             },
		{BlendFactor::OneMinusSrc1Color,        vk::BlendFactor::eOneMinusSrc1Color,     },
		{BlendFactor::Src1Alpha,                vk::BlendFactor::eSrc1Alpha,             },
		{BlendFactor::OneMinusSrc1Alpha,        vk::BlendFactor::eOneMinusSrc1Alpha,     },
	} };

util::ValueRemapper<RenderState::ColorComponentMask, vk::ColorComponentFlagBits, true> RenderStateVk::s_colorComponentMask2Vk{ {
		{ ColorComponentMask::R, vk::ColorComponentFlagBits::eR	},
		{ ColorComponentMask::G, vk::ColorComponentFlagBits::eG	},
		{ ColorComponentMask::B, vk::ColorComponentFlagBits::eB	},
		{ ColorComponentMask::A, vk::ColorComponentFlagBits::eA	},
	} };

void RenderStateVk::Init()
{
	_state = ResourceState::ShaderRead;
}

void RenderStateVk::FillViewportState(vk::PipelineViewportStateCreateInfo &viewportState, std::vector<vk::Viewport> &viewports, std::vector<vk::Rect2D> &scissors)
{
	FillViewports(viewports);

	scissors.emplace_back();
	scissors.back()
		.setOffset(vk::Offset2D(_scissor._min.x, _scissor._min.y))
		.setExtent(vk::Extent2D(_scissor.GetSize().x, _scissor.GetSize().y));

	viewportState
		.setViewportCount(1)
		.setPViewports(&viewports.back())
		.setScissorCount(1)
		.setPScissors(&scissors.back());
}

void RenderStateVk::FillRasterizationState(vk::PipelineRasterizationStateCreateInfo &rasterState)
{
	rasterState
		.setCullMode(s_cullMask2Vk.ToDst(_cullState._cullMask))
		.setFrontFace(s_frontFace2Vk.ToDst(_cullState._front))
		.setDepthBiasEnable(_depthBias._enable)
		.setDepthBiasConstantFactor(_depthBias._constantFactor)
		.setDepthBiasClamp(_depthBias._clamp)
		.setDepthBiasSlopeFactor(_depthBias._slopeFactor);
}

void RenderStateVk::FillMultisampleState(vk::PipelineMultisampleStateCreateInfo &multisampleState, std::vector<uint32_t> &sampleMask)
{
	// leave it default
}

void RenderStateVk::FillDepthStencilState(vk::PipelineDepthStencilStateCreateInfo &depthState)
{
	depthState
		.setDepthTestEnable(_depthState._depthTestEnable)
		.setDepthWriteEnable(_depthState._depthWriteEnable)
		.setDepthCompareOp(s_compareFunc2Vk.ToDst(_depthState._depthCompareFunc))
		.setDepthBoundsTestEnable(_depthState._depthBoundsTestEnable)
		.setMinDepthBounds(_depthState._minDepthBounds)
		.setMaxDepthBounds(_depthState._maxDepthBounds)
		.setStencilTestEnable(_stencilEnable);
	FillStencilOpState(_stencilState[0], depthState.front);
	FillStencilOpState(_stencilState[1], depthState.back);
}

void RenderStateVk::FillBlendState(vk::PipelineColorBlendStateCreateInfo &blendState, std::vector<vk::PipelineColorBlendAttachmentState> &attachmentBlends)
{
	for (auto &blend : _blendStates) {
		attachmentBlends.resize(attachmentBlends.size() + 1);
		attachmentBlends.back()
			.setBlendEnable(blend._blendEnable)
			.setSrcColorBlendFactor(s_blendFactor2Vk.ToDst(blend._srcColorBlendFactor))
			.setDstColorBlendFactor(s_blendFactor2Vk.ToDst(blend._dstColorBlendFactor))
			.setColorBlendOp(s_blendFunc2Vk.ToDst(blend._colorBlendFunc))
			.setSrcAlphaBlendFactor(s_blendFactor2Vk.ToDst(blend._srcAlphaBlendFactor))
			.setDstAlphaBlendFactor(s_blendFactor2Vk.ToDst(blend._dstAlphaBlendFactor))
			.setAlphaBlendOp(s_blendFunc2Vk.ToDst(blend._alphaBlendFunc))
			.setColorWriteMask(s_colorComponentMask2Vk.ToDst(blend._colorWriteMask));
	}

	blendState
		.setAttachmentCount(static_cast<uint32_t>(attachmentBlends.size()))
		.setPAttachments(attachmentBlends.data())
		.setBlendConstants({_blendColor[0], _blendColor[1], _blendColor[2], _blendColor[3]});
}

void RenderStateVk::FillDynamicState(vk::PipelineDynamicStateCreateInfo &dynamicState, std::vector<vk::DynamicState> &dynamicStates)
{
	dynamicStates.push_back(vk::DynamicState::eViewport);
	
	dynamicState
		.setDynamicStateCount(static_cast<uint32_t>(dynamicStates.size()))
		.setPDynamicStates(dynamicStates.data());
}

void RenderStateVk::FillStencilOpState(StencilFuncState const &src, vk::StencilOpState &dst)
{
	dst
		.setFailOp(s_stencilFunc2Vk.ToDst(src._failFunc))
		.setPassOp(s_stencilFunc2Vk.ToDst(src._passFunc))
		.setDepthFailOp(s_stencilFunc2Vk.ToDst(src._depthFailFunc))
		.setCompareOp(s_compareFunc2Vk.ToDst(src._compareFunc))
		.setWriteMask(src._writeMask)
		.setReference(src._reference)
		.setCompareMask(src._compareMask);
}

void RenderStateVk::FillViewports(std::vector<vk::Viewport> &viewports)
{
	viewports.emplace_back();
	viewports.back()
		.setX(_viewport._rect._min.x)
		.setY(_viewport._rect._min.y)
		.setWidth(_viewport._rect.GetSize().x)
		.setHeight(_viewport._rect.GetSize().y)
		.setMinDepth(_viewport._minDepth)
		.setMaxDepth(_viewport._maxDepth);
}

NAMESPACE_END(gr1)

