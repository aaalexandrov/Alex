#include "render_commands_vk.h"
#include "device_vk.h"
#include "image_vk.h"
#include "shader_vk.h"
#include "buffer_vk.h"
#include "sampler_vk.h"
#include "execution_queue_vk.h"
#include "render_state_vk.h"
#include "render_pipeline_vk.h"
#include "../graphics_exception.h"
#include "rttr/registration.h"

NAMESPACE_BEGIN(gr1)

RTTR_REGISTRATION
{
	using namespace rttr;
	registration::class_<PipelineDrawCommandVk>("PipelineDrawCommandVk")(metadata(RttrDiscriminator::Tag, type::get<DeviceVk>()))
		.constructor<Device&>()(policy::ctor::as_raw_ptr);
	registration::class_<RenderDrawCommandVk>("RenderDrawCommandVk")(metadata(RttrDiscriminator::Tag, type::get<DeviceVk>()))
		.constructor<Device&>()(policy::ctor::as_raw_ptr);
}


PipelineDrawCommandVk::PipelineDrawCommandVk(Device &device) 
	: PipelineDrawCommand(device)
{
}

void PipelineDrawCommandVk::PrepareToRecord(CommandPrepareInfo &prepareInfo)
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	CommandPrepareInfoVk *prepInfoVk = static_cast<CommandPrepareInfoVk*>(&prepareInfo);
	RenderPipelineVk *pipelineVk = static_cast<RenderPipelineVk*>(_pipeline.get());

	if (pipelineVk->GetResourceState() != ResourceState::ShaderRead) {
		_cmdDraw.reset();
		_descriptorSet.reset();
	}

	UpdateDescriptorSets();

	DynamicState dynState = GetDynamicState(*prepInfoVk);
	if (_cmdDraw && _recordedDynState == dynState)
		return;

	_cmdDraw = deviceVk->GraphicsQueue().AllocateCmdBuffer(vk::CommandBufferLevel::eSecondary);

	vk::CommandBufferBeginInfo beginInfo;
	beginInfo
		.setFlags(vk::CommandBufferUsageFlagBits::eRenderPassContinue)
		.setPInheritanceInfo(&prepInfoVk->_cmdInheritInfo);

	std::lock_guard<CmdBufferVk> drawLock(_cmdDraw);

	_cmdDraw->begin(beginInfo);

	_cmdDraw->bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineVk->_pipeline->Get());

	RecordDynamicState(dynState);

	std::array<vk::DescriptorSet, 1> descriptorSets{*_descriptorSet};
	_cmdDraw->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineVk->_pipeline->_pipelineLayout->Get(), 0, descriptorSets, nullptr);

	std::vector<vk::Buffer> vertBuffers;
	std::vector<size_t> vertBufferOffsets;
	for (int32_t bufIndex : pipelineVk->GetVertexBufferIndices()) {
		BufferData const *bufData = _resources[bufIndex].GetBufferData();
		BufferVk *bufferVk = static_cast<BufferVk*>(bufData->_buffer.get());
		vertBuffers.push_back(*bufferVk->_buffer);
		vertBufferOffsets.push_back(bufData->_offset);
	}
	_cmdDraw->bindVertexBuffers(0, vertBuffers, vertBufferOffsets);

	BufferData const *indexBufData = _resources[pipelineVk->GetIndexBufferIndex()].GetBufferData();
	if (indexBufData->_buffer) {
		BufferVk *indexBufferVk = static_cast<BufferVk*>(indexBufData->_buffer.get());
		vk::IndexType indexType = indexBufferVk->GetVkIndexType(indexBufferVk->GetBufferLayout().get());
		_cmdDraw->bindIndexBuffer(*indexBufferVk->_buffer, indexBufData->_offset, indexType);

		_cmdDraw->drawIndexed(_drawCounts._indexCount, _drawCounts._instanceCount, _drawCounts._firstIndex, _drawCounts._vertexOffset, _drawCounts._firstInstance);
	} else {
		_cmdDraw->draw(_drawCounts._indexCount, _drawCounts._instanceCount, _drawCounts._firstIndex, _drawCounts._firstInstance);
	}

	_cmdDraw->end();
}

void PipelineDrawCommandVk::Record(CommandRecordInfo &recordInfo)
{
	CommandRecordInfoVk *recInfoVk = static_cast<CommandRecordInfoVk*>(&recordInfo);

	recInfoVk->_secondaryCmds->push_back(*_cmdDraw);
}

void PipelineDrawCommandVk::UpdateDescriptorSets()
{
	if (_descriptorSet && _descriptorSetValid)
		return;

	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	RenderPipelineVk *pipelineVk = static_cast<RenderPipelineVk*>(_pipeline.get());

	std::vector<vk::WriteDescriptorSet> setWrites;
	std::vector<vk::DescriptorBufferInfo> bufferInfos;
	std::vector<vk::DescriptorImageInfo> samplerInfos;
	// reserve enough elements so that the arrays won't be reallocated, we're storing addresses of elements below
	size_t maxDescriptors = _resources.size() * ShaderKind::Count;
	bufferInfos.reserve(maxDescriptors);
	samplerInfos.reserve(maxDescriptors);

	for (uint32_t r = pipelineVk->GetFirstUniformResourceIndex(); r < _resources.size(); ++r) {
		ResourceData &resData = _resources[r];
		RenderPipeline::ResourceInfo const &resInfo = pipelineVk->GetResourceInfos()[r];
		switch (resInfo._kind) {
			case Shader::Parameter::UniformBuffer: {
				BufferData const *bufData = resData.GetBufferData();
				BufferVk *bufferVk = static_cast<BufferVk*>(bufData->_buffer.get());
				bufferInfos.emplace_back();
				bufferInfos.back()
					.setBuffer(*bufferVk->_buffer)
					.setOffset(bufData->_offset)
					.setRange(bufData->_size);

				for (uint32_t i = 0; i < resInfo._binding.size(); ++i) {
					if (resInfo._binding[i] == ~0u)
						break;
					setWrites.emplace_back();
					setWrites.back()
						.setDstSet(*_descriptorSet)
						.setDstBinding(resInfo._binding[i])
						.setDstArrayElement(0)
						.setDescriptorCount(1)
						.setDescriptorType(vk::DescriptorType::eUniformBuffer)
						.setPBufferInfo(&bufferInfos.back());
				}
				break;
			}
			case Shader::Parameter::Sampler: {
				SamplerData const *samplerData = resData.GetSamplerData();
				SamplerVk *samplerVk = static_cast<SamplerVk*>(samplerData->_sampler.get());
				ImageVk *imageVk = static_cast<ImageVk*>(samplerData->_image.get());
				samplerInfos.emplace_back();
				samplerInfos.back()
					.setSampler(*samplerVk->_sampler)
					.setImageView(*imageVk->_view)
					.setImageLayout(imageVk->GetStateInfo(ResourceState::ShaderRead)._layout);

				for (uint32_t i = 0; i < resInfo._binding.size(); ++i) {
					if (resInfo._binding[i] == ~0u)
						continue;
					setWrites.emplace_back();
					setWrites.back()
						.setDstSet(*_descriptorSet)
						.setDstBinding(resInfo._binding[i])
						.setDstArrayElement(0)
						.setDescriptorCount(1)
						.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
						.setPImageInfo(&samplerInfos.back());
				}
				break;
			}
		}
	}
	if (setWrites.size())
		deviceVk->_device->updateDescriptorSets(setWrites, nullptr);

	_descriptorSetValid = true;
}

DynamicState PipelineDrawCommandVk::GetDynamicState(CommandPrepareInfoVk &prepareInfo)
{
	DynamicState dynState;
	RenderState::Viewport const &viewport = _pipeline->GetRenderState()->GetViewport();
	if (!viewport._rect.IsEmpty()) {
		dynState._viewport = RenderStateVk::GetVkViewport(viewport);
	} else {
		dynState._viewport = prepareInfo._viewport;
	}
	return dynState;
}

void PipelineDrawCommandVk::RecordDynamicState(DynamicState const &dynState)
{
	std::vector<vk::Viewport> viewports;
	viewports.push_back(dynState._viewport);
	_cmdDraw->setViewport(0, viewports);

	_recordedDynState = dynState;
}

RenderDrawCommandVk::RenderDrawCommandVk(Device &device)
	: RenderDrawCommand(device)
{
}

void RenderDrawCommandVk::Clear()
{
	_cmdDraw.reset();
	_pipeline.reset();
	RenderDrawCommand::Clear();
}

void RenderDrawCommandVk::SetRenderState(std::shared_ptr<RenderState> const &renderState)
{
	_cmdDraw.reset();
	_pipeline.reset();
	RenderDrawCommand::SetRenderState(renderState);
}

void RenderDrawCommandVk::SetShader(std::shared_ptr<Shader> const &shader)
{
	_cmdDraw.reset();
	_pipeline.reset();
	RenderDrawCommand::SetShader(shader);
}

int RenderDrawCommandVk::AddBuffer(std::shared_ptr<Buffer> const &buffer, util::StrId shaderId, size_t offset, bool frequencyInstance, std::shared_ptr<util::LayoutElement> const &overrideLayout)
{
	_cmdDraw.reset();
	_descriptorSetValid = false;
	if (!!(buffer->GetUsage() & (Buffer::Usage::Vertex | Buffer::Usage::Index))) {
		_pipeline.reset();
	}
	return RenderDrawCommand::AddBuffer(buffer, shaderId, offset, frequencyInstance, overrideLayout);
}

void RenderDrawCommandVk::RemoveBuffer(int bufferIndex)
{
	_cmdDraw.reset();
	_descriptorSetValid = false;
	if (!!(_buffers[bufferIndex]._buffer->GetUsage() & (Buffer::Usage::Vertex | Buffer::Usage::Index))) {
		_descriptorSet.reset();
		_pipeline.reset();
	}
	RenderDrawCommand::RemoveBuffer(bufferIndex);
}

void RenderDrawCommandVk::SetPrimitiveKind(PrimitiveKind primitiveKind)
{
	if (primitiveKind != _primitiveKind) {
		_cmdDraw.reset();
		_pipeline.reset();
	}
	RenderDrawCommand::SetPrimitiveKind(primitiveKind);
}

void RenderDrawCommandVk::SetDrawCounts(uint32_t indexCount, uint32_t firstIndex, uint32_t instanceCount, uint32_t firstInstance, uint32_t vertexOffset)
{
	_cmdDraw.reset();
	RenderDrawCommand::SetDrawCounts(indexCount, firstIndex, instanceCount, firstInstance, vertexOffset);
}

int RenderDrawCommandVk::AddSampler(std::shared_ptr<Sampler> const &sampler, std::shared_ptr<Image> const &image, util::StrId shaderId)
{
	_cmdDraw.reset();
	_descriptorSetValid = false;
	return RenderDrawCommand::AddSampler(sampler, image, shaderId);
}

void RenderDrawCommandVk::RemoveSampler(int samplerIndex)
{
	_cmdDraw.reset();
	_descriptorSetValid = false;
	RenderDrawCommand::RemoveSampler(samplerIndex);
}

void RenderDrawCommandVk::PrepareToRecord(CommandPrepareInfo &prepareInfo)
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	CommandPrepareInfoVk *prepInfoVk = static_cast<CommandPrepareInfoVk*>(&prepareInfo);

	if (_pipelineRenderStateVersion != _renderState->GetStateDataVersion()) {
		_cmdDraw.reset();
		_pipeline.reset();
	}

	if (_cmdDraw && _recordedViewport == prepInfoVk->_viewport)
		return;

	PreparePipeline(static_cast<RenderPassVk*>(prepInfoVk->_renderPass), prepInfoVk->_cmdInheritInfo.subpass);
	PrepareDescriptorSets();

	UpdateDescriptorSets();

	_cmdDraw = deviceVk->GraphicsQueue().AllocateCmdBuffer(vk::CommandBufferLevel::eSecondary);

	vk::CommandBufferBeginInfo beginInfo;
	beginInfo
		.setFlags(vk::CommandBufferUsageFlagBits::eRenderPassContinue)
		.setPInheritanceInfo(&prepInfoVk->_cmdInheritInfo);

	std::lock_guard<CmdBufferVk> drawLock(_cmdDraw);

	_cmdDraw->begin(beginInfo);

	_cmdDraw->bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline->Get());

	SetDynamicState(*prepInfoVk);
	_recordedViewport = prepInfoVk->_viewport;

	std::array<vk::DescriptorSet, 1> descriptorSets{*_descriptorSet};
	_cmdDraw->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipeline->_pipelineLayout->Get(), 0, descriptorSets, nullptr);

	vk::Buffer indexBuffer;
	vk::IndexType indexType;
	size_t indexOffset;
	std::vector<vk::Buffer> vertBuffers;
	std::vector<size_t> vertBufferOffsets;
	for (auto &bufData : _buffers) {
		BufferVk *bufferVk = static_cast<BufferVk*>(bufData._buffer.get());
		if (bufData.IsVertex()) {
			// vertex buffer bindings are implicit, counting from 0 in the order the buffers appear in the array
			// pipeline store uses the same logic
			vertBuffers.push_back(*bufferVk->_buffer);
			vertBufferOffsets.push_back(bufData._offset);
		}
		if (bufData.IsIndex()) {
			ASSERT(!indexBuffer);
			indexBuffer = *bufferVk->_buffer;
			indexType = bufferVk->GetVkIndexType(bufData.GetBufferLayout().get());
			indexOffset = bufData._offset;
		}
	}
	_cmdDraw->bindVertexBuffers(0, vertBuffers, vertBufferOffsets);
	if (indexBuffer) {
		_cmdDraw->bindIndexBuffer(indexBuffer, indexOffset, indexType);

		_cmdDraw->drawIndexed(_drawCounts._indexCount, _drawCounts._instanceCount, _drawCounts._firstIndex, _drawCounts._vertexOffset, _drawCounts._firstInstance);
	} else {
		_cmdDraw->draw(_drawCounts._indexCount, _drawCounts._instanceCount, _drawCounts._firstIndex, _drawCounts._firstInstance);
	}

	_cmdDraw->end();
}

void RenderDrawCommandVk::Record(CommandRecordInfo &recordInfo)
{
	CommandRecordInfoVk *recInfoVk = static_cast<CommandRecordInfoVk*>(&recordInfo);

	recInfoVk->_secondaryCmds->push_back(*_cmdDraw);
}

void RenderDrawCommandVk::PreparePipeline(RenderPassVk *renderPass, uint32_t subpass)
{
	if (_pipeline)
		return;

	DeviceVk *deviceVk = GetDevice<DeviceVk>();

	PipelineInfo pipeInfo;
	for (int i = 0; i < _shaders.size(); ++i) {
		pipeInfo._shaders[i] = static_cast<ShaderVk*>(_shaders[i].get());
	}

	for (int i = 0; i < _buffers.size(); ++i) {
		if (!_buffers[i].IsVertex())
			continue;

		VertexBufferLayout bufLayout;
		bufLayout._bufferLayout = _buffers[i].GetBufferLayout();
		bufLayout._frequencyInstance = _buffers[i]._frequencyInstance;
		pipeInfo._vertexBufferLayouts.push_back(bufLayout);
	}

	pipeInfo._renderStateData = &_renderState->GetStateData();
	pipeInfo._primitiveKind = _primitiveKind;
	pipeInfo._renderPass = renderPass;
	pipeInfo._subpass = subpass;

	_pipeline = deviceVk->_pipelineStore.GetPipeline(pipeInfo);
	_pipelineRenderStateVersion = _renderState->GetStateDataVersion();

	_descriptorSet.reset();
}

void RenderDrawCommandVk::PrepareDescriptorSets()
{
	if (_descriptorSet)
		return;
	_descriptorSet = _pipeline->AllocateDescriptorSet();
	_descriptorSetValid = false;
}

uint32_t RenderDrawCommandVk::GetMaxBuffersDescriptorCount()
{
	return static_cast<uint32_t>(_buffers.size()) * ShaderKind::Count;
}

uint32_t RenderDrawCommandVk::GetMaxSamplersDescriptorCount()
{
	return static_cast<uint32_t>(_samplers.size()) * ShaderKind::Count;
}

void RenderDrawCommandVk::UpdateDescriptorSets()
{
	if (_descriptorSetValid)
		return;

	DeviceVk *deviceVk = GetDevice<DeviceVk>();

	std::vector<vk::WriteDescriptorSet> setWrites;

	std::vector<vk::DescriptorBufferInfo> bufferInfos;
	// reserve enough elements so that the array won't be reallocated, we're storing addresses of elements below
	bufferInfos.reserve(GetMaxBuffersDescriptorCount());
	for (auto &bufData : _buffers) {
		if (!!(bufData._buffer->GetUsage() & Buffer::Usage::Uniform)) {
			BufferVk *bufferVk = static_cast<BufferVk*>(bufData._buffer.get());
			bufferInfos.emplace_back();
			bufferInfos.back()
				.setBuffer(*bufferVk->_buffer)
				.setOffset(bufData._offset)
				.setRange(bufData.GetBufferLayout()->GetSize());

			for (uint32_t i = 0; i < _shaders.size(); ++i) {
				if (bufData._bindings[i] == ~0u)
					continue;
				setWrites.emplace_back();
				setWrites.back()
					.setDstSet(*_descriptorSet)
					.setDstBinding(bufData._bindings[i])
					.setDstArrayElement(0)
					.setDescriptorCount(1)
					.setDescriptorType(vk::DescriptorType::eUniformBuffer)
					.setPBufferInfo(&bufferInfos.back());
			}
		}
	}

	std::vector<vk::DescriptorImageInfo> samplerInfos;
	samplerInfos.reserve(GetMaxSamplersDescriptorCount());
	for (auto &samplerData : _samplers) {
		SamplerVk *samplerVk = static_cast<SamplerVk*>(samplerData._sampler.get());
		ImageVk *imageVk = static_cast<ImageVk*>(samplerData._image.get());
		samplerInfos.emplace_back();
		samplerInfos.back()
			.setSampler(*samplerVk->_sampler)
			.setImageView(*imageVk->_view)
			.setImageLayout(imageVk->GetStateInfo(ResourceState::ShaderRead)._layout);

		for (uint32_t i = 0; i < _shaders.size(); ++i) {
			if (samplerData._bindings[i] == ~0u)
				continue;
			setWrites.emplace_back();
			setWrites.back()
				.setDstSet(*_descriptorSet)
				.setDstBinding(samplerData._bindings[i])
				.setDstArrayElement(0)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
				.setPImageInfo(&samplerInfos.back());
		}
	}

	if (setWrites.size())
		deviceVk->_device->updateDescriptorSets(setWrites, nullptr);

	_descriptorSetValid = true;
}

void RenderDrawCommandVk::SetDynamicState(CommandPrepareInfoVk &prepareInfo)
{
	std::vector<vk::Viewport> viewports;

	RenderState::Viewport const &viewport = _renderState->GetViewport();
	if (!viewport._rect.IsEmpty()) {
		RenderStateVk::FillViewports(_renderState->GetStateData(), viewports);
	} else {
		viewports.push_back(prepareInfo._viewport);
	}

	_cmdDraw->setViewport(0, viewports);
}


NAMESPACE_END(gr1)

