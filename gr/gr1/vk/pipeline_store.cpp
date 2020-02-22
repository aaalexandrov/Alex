#include "pipeline_store.h"
#include "device_vk.h"
#include "shader_vk.h"
#include "render_state_vk.h"
#include "render_pass_vk.h"

NAMESPACE_BEGIN(gr1)

size_t VertexBufferLayout::GetHash() const
{
	size_t hash = _bufferLayout->GetArrayElement()->GetHash();
	hash = util::GetHash(_binding, hash);
	hash = util::GetHash(_frequencyInstance, hash);
	return hash;
}

bool VertexBufferLayout::operator==(VertexBufferLayout const &other) const
{
	return *_bufferLayout->GetArrayElement() == *other._bufferLayout->GetArrayElement()
		&& _binding == other._binding
		&& _frequencyInstance == other._frequencyInstance;
}

void PipelineLayoutVk::Init(DeviceVk &deviceVk, ShaderKindsArray<ShaderVk*> &shaders)
{
	std::vector<vk::DescriptorSetLayoutBinding> layoutBindings;
	for (int i = 0; i < shaders.size(); ++i) {
		if (!shaders[i])
			continue;
		_shaders[i] = shaders[i]->AsSharedPtr<ShaderVk>();
		shaders[i]->FillDescriptorSetLayoutBindings(layoutBindings);
	}

	vk::DescriptorSetLayoutCreateInfo setInfo;
	setInfo
		.setBindingCount(static_cast<uint32_t>(layoutBindings.size()))
		.setPBindings(layoutBindings.data());
	_descriptorSetLayout = deviceVk._device->createDescriptorSetLayoutUnique(setInfo, deviceVk.AllocationCallbacks());

	_descriptorSetStore.Init(deviceVk, layoutBindings, 256);

	vk::PipelineLayoutCreateInfo pipeInfo;
	pipeInfo
		.setSetLayoutCount(1)
		.setPSetLayouts(&*_descriptorSetLayout);
	_pipelineLayout = deviceVk._device->createPipelineLayoutUnique(pipeInfo, deviceVk.AllocationCallbacks());
}

size_t PipelineInfo::GetHash()
{
	size_t hash = util::GetHash(_shaders);
	hash = util::GetHash(_vertexBufferLayouts, hash);
	hash = 31 * hash + _renderStateData->GetHash();
	hash = util::GetHash(_primitiveKind, hash);
	hash = util::GetHash(_renderPass, hash);
	hash = util::GetHash(_subpass, hash);
	return hash;
}

bool PipelineInfo::operator==(PipelineInfo const &other) const
{
	bool eq = _shaders == other._shaders;
	eq = eq && _vertexBufferLayouts == other._vertexBufferLayouts;
	eq = eq && *_renderStateData == *other._renderStateData;
	eq = eq && _primitiveKind == other._primitiveKind;
	eq = eq && _renderPass == other._renderPass;
	eq = eq && _subpass == other._subpass;

	return eq;
}


void RecordedPipeline::CopyToOwnData()
{
	PipelineInfo *pipelineInfo = _pipelineInfo;
	_ownPipelineInfo = std::make_unique<PipelineInfo>(*pipelineInfo);
	_ownStateData = std::make_unique<RenderState::StateData>(*pipelineInfo->_renderStateData);
	_pipelineInfo = _ownPipelineInfo.get();
	_pipelineInfo->_renderStateData = _ownStateData.get();
	_weakRenderPass = _pipelineInfo->_renderPass->AsSharedPtr<RenderPassVk>();
}

bool RecordedPipeline::IsValid() const
{
	return _pipelineInfo->_renderPass == _weakRenderPass.lock().get();
}

bool RecordedPipeline::operator==(RecordedPipeline const &other) const
{
	return *_pipelineInfo == *other._pipelineInfo;
}

void PipelineStore::Init(DeviceVk &deviceVk)
{
	_deviceVk = &deviceVk;

	vk::PipelineCacheCreateInfo cacheInfo;
	_deviceVk->_device->createPipelineCacheUnique(cacheInfo, _deviceVk->AllocationCallbacks());
}

util::ValueRemapper<PrimitiveKind, vk::PrimitiveTopology> s_PrimitiveKind2Topology = { {
		{ PrimitiveKind::TriangleList, vk::PrimitiveTopology::eTriangleList },
		{ PrimitiveKind::TriangleStrip, vk::PrimitiveTopology::eTriangleStrip },
	} };

std::shared_ptr<PipelineVk> PipelineStore::GetPipeline(PipelineInfo &pipelineInfo)
{
	std::lock_guard lock(_mutex);

	RecordedPipeline pipelineKey(&pipelineInfo);
	auto it = _pipelines.find(pipelineKey);
	auto itLayout = GetExistingPipelineLayout(pipelineInfo._shaders);
	if (it != _pipelines.end()) {
		bool valid = it->first.IsValid();
		valid = valid && !it->second.expired();
		valid = valid && itLayout != _pipelineLayouts.end();
		if (!valid) {
			_pipelines.erase(it);
			it = _pipelines.end();
		}
	}

	std::shared_ptr<PipelineVk> pipeline;
	if (it == _pipelines.end()) {
		std::shared_ptr<PipelineLayoutVk> layout;
		if (itLayout == _pipelineLayouts.end()) {
			layout = AddPipelineLayout(pipelineInfo._shaders);
		} else {
			layout = itLayout->second;
		}
		pipelineKey.CopyToOwnData();
		ASSERT(*pipelineKey._pipelineInfo == pipelineInfo);
		ASSERT(util::GetHash(pipelineKey) == pipelineInfo.GetHash());
		pipeline = AllocatePipeline(layout, pipelineInfo);
		auto pair = std::pair<RecordedPipeline, std::weak_ptr<PipelineVk>>(std::move(pipelineKey), pipeline);
		_pipelines.insert(std::move(pair));
	} else {
		pipeline = it->second.lock();
	}

	return pipeline;
}

std::shared_ptr<PipelineVk> PipelineStore::AllocatePipeline(std::shared_ptr<PipelineLayoutVk> const &pipelineLayout, PipelineInfo &pipelineInfo)
{
	std::vector<vk::PipelineShaderStageCreateInfo> shaderStageInfos;
	std::vector<vk::VertexInputAttributeDescription> vertexInputAttribDescs;
	std::vector<vk::VertexInputBindingDescription> vertexInputBindingDescs;
	for (auto shaderVk : pipelineInfo._shaders) {
		shaderStageInfos.push_back(shaderVk->GetPipelineShaderStageCreateInfo());
		if (shaderVk->GetShaderKind() == ShaderKind::Vertex) {
			util::LayoutElement *shaderVertLayout = shaderVk->GetVertexLayout().get();

			ASSERT(shaderVertLayout->GetKind() == util::LayoutElement::Kind::Struct);
			for (uint32_t i = 0; i < shaderVertLayout->GetStructFieldCount(); ++i) {
				util::LayoutElement const *shaderAttrElem = shaderVertLayout->GetStructFieldElement(i);

				std::string attrName = shaderVertLayout->GetStructFieldName(i);
				size_t attrOffset;
				util::LayoutElement const *attrElem;
				int bufIndex = GetVertexLayoutIndex(attrName, pipelineInfo._vertexBufferLayouts, attrOffset, attrElem);
				if (bufIndex < 0) {
					ASSERT(!"Shader vertex attribute missing from supplied buffers!");
					continue;
				}

				uint32_t binding = pipelineInfo._vertexBufferLayouts[bufIndex]._binding;
				uint32_t location = static_cast<uint32_t>(shaderAttrElem->GetUserData());
				vk::Format format = Type2VkFormat.at(attrElem->GetValueType());

				vertexInputAttribDescs.emplace_back();
				vertexInputAttribDescs.back()
					.setLocation(location)
					.setBinding(binding)
					.setFormat(format)
					.setOffset(static_cast<uint32_t>(attrOffset));
			}

			for (int i = 0; i < pipelineInfo._vertexBufferLayouts.size(); ++i) {
				auto bufLayout = pipelineInfo._vertexBufferLayouts[i];

				vertexInputBindingDescs.emplace_back();
				vertexInputBindingDescs.back()
					.setStride(static_cast<uint32_t>(bufLayout._bufferLayout->GetArrayElement()->GetSize()))
					.setBinding(bufLayout._binding)
					.setInputRate(bufLayout._frequencyInstance ? vk::VertexInputRate::eInstance : vk::VertexInputRate::eVertex);
			}
		}
	}

	vk::PipelineVertexInputStateCreateInfo vertexInputStateInfo;
	vertexInputStateInfo
		.setVertexAttributeDescriptionCount(static_cast<uint32_t>(vertexInputAttribDescs.size()))
		.setPVertexAttributeDescriptions(vertexInputAttribDescs.data())
		.setVertexBindingDescriptionCount(static_cast<uint32_t>(vertexInputBindingDescs.size()))
		.setPVertexBindingDescriptions(vertexInputBindingDescs.data());

	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
	inputAssemblyInfo
		.setTopology(s_PrimitiveKind2Topology.ToDst(pipelineInfo._primitiveKind));

	std::vector<vk::Viewport> viewports;
	std::vector<vk::Rect2D> scissors;
	vk::PipelineViewportStateCreateInfo viewportState;
	RenderStateVk::FillViewportState(*pipelineInfo._renderStateData, viewportState, viewports, scissors);

	vk::PipelineRasterizationStateCreateInfo rasterizationState;
	RenderStateVk::FillRasterizationState(*pipelineInfo._renderStateData, rasterizationState);

	std::vector<uint32_t> sampleMask;
	vk::PipelineMultisampleStateCreateInfo multisampleState;
	RenderStateVk::FillMultisampleState(*pipelineInfo._renderStateData, multisampleState, sampleMask);

	vk::PipelineDepthStencilStateCreateInfo depthStencilState;
	RenderStateVk::FillDepthStencilState(*pipelineInfo._renderStateData, depthStencilState);

	std::vector<vk::PipelineColorBlendAttachmentState> attachmentBlends;
	vk::PipelineColorBlendStateCreateInfo blendState;
	RenderStateVk::FillBlendState(*pipelineInfo._renderStateData, blendState, attachmentBlends);

	std::vector<vk::DynamicState> dynamicStates;
	vk::PipelineDynamicStateCreateInfo dynamicState;
	RenderStateVk::FillDynamicState(*pipelineInfo._renderStateData, dynamicState, dynamicStates);

	vk::GraphicsPipelineCreateInfo pipeCreateInfo;
	pipeCreateInfo
		.setStageCount(static_cast<uint32_t>(shaderStageInfos.size()))
		.setPStages(shaderStageInfos.data())
		.setPVertexInputState(&vertexInputStateInfo)
		.setPInputAssemblyState(&inputAssemblyInfo)
		.setPViewportState(&viewportState)
		.setPRasterizationState(&rasterizationState)
		.setPMultisampleState(&multisampleState)
		.setPDepthStencilState(&depthStencilState)
		.setPColorBlendState(&blendState)
		.setPDynamicState(&dynamicState)
		.setLayout(pipelineLayout->Get())
		.setRenderPass(pipelineInfo._renderPass->GetVkRenderPass())
		.setSubpass(pipelineInfo._subpass)
		.setBasePipelineIndex(-1);

	vk::UniquePipeline pipeline = _deviceVk->_device->createGraphicsPipelineUnique(*_pipelineCache, pipeCreateInfo, _deviceVk->AllocationCallbacks());

	return std::make_shared<PipelineVk>(std::move(pipelineLayout), std::move(pipeline));
}

int PipelineStore::GetVertexLayoutIndex(std::string attribName, std::vector<VertexBufferLayout> &bufferLayouts, size_t &attribOffset, util::LayoutElement const *&attribLayout)
{
	for (int i = 0; i < bufferLayouts.size(); ++i) {
		auto &bufLayout = bufferLayouts[i];

		util::LayoutElement const *vertDesc = bufLayout._bufferLayout->GetArrayElement();
		ASSERT(vertDesc->GetKind() == util::LayoutElement::Kind::Struct);
		size_t elemIndex = vertDesc->GetStructFieldIndex(attribName);
		attribLayout = vertDesc->GetElement(elemIndex);
		if (!attribLayout)
			continue;
		attribOffset = vertDesc->GetStructFieldOffset(elemIndex);
		return i;
	}
	attribOffset = ~0ul;
	return -1;
}

PipelineStore::PipelineLayoutsMap::iterator PipelineStore::GetExistingPipelineLayout(ShaderKindsArray<ShaderVk*> &shaders)
{
	auto it = _pipelineLayouts.find(shaders);
	if (it != _pipelineLayouts.end()) {
		// it's possible that some of the original shaders were released, and then a new shader was allocated at the same address
		// so a tuple of shader addresses is not enough to ascertain we're asking for the same actual shaders that were recorded
		// so we check that the weak pointers stored in the PipelineLayoutVk object dereference to what we're looking for
		bool valid = true;
		for (int i = 0; i < shaders.size(); ++i) {
			if (it->second->_shaders[i].lock().get() != shaders[i]) {
				valid = false;
				break;
			}
		}
		if (!valid) {
			_pipelineLayouts.erase(it);
			it = _pipelineLayouts.end();
		}
	}
	return it;
}

std::shared_ptr<PipelineLayoutVk> PipelineStore::AddPipelineLayout(ShaderKindsArray<ShaderVk*> &shaders)
{
	auto layoutVk = std::make_shared<PipelineLayoutVk>();
	layoutVk->Init(*_deviceVk, shaders);
	auto it = _pipelineLayouts.insert(std::make_pair(shaders, std::move(layoutVk))).first;

	return it->second;
}

DescriptorSetVk PipelineVk::AllocateDescriptorSet()
{
	return _pipelineLayout->_descriptorSetStore.AllocateDescriptorSet(*_pipelineLayout->_descriptorSetLayout);
}

NAMESPACE_END(gr1)

