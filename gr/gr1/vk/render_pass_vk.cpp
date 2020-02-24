#include "render_pass_vk.h"
#include "device_vk.h"
#include "image_vk.h"
#include "shader_vk.h"
#include "buffer_vk.h"
#include "sampler_vk.h"
#include "execution_queue_vk.h"
#include "render_state_vk.h"
#include "../graphics_exception.h"
#include "rttr/registration.h"

NAMESPACE_BEGIN(gr1)

RTTR_REGISTRATION
{
	using namespace rttr;
	registration::class_<RenderDrawCommandVk>("RenderDrawCommandVk")(metadata(RttrDiscriminator::Tag, type::get<DeviceVk>()))
		.constructor<Device&>()(policy::ctor::as_raw_ptr);
		registration::class_<RenderPassVk>("RenderPassVk")(metadata(RttrDiscriminator::Tag, type::get<DeviceVk>()))
		.constructor<Device&>()(policy::ctor::as_raw_ptr);
}

CommandPrepareInfoVk::CommandPrepareInfoVk(RenderPassVk *renderPassVk)
	: CommandPrepareInfo(renderPassVk)
{
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
				.setRange(bufferVk->GetSize() - bufData._offset);

			for (uint32_t i = 0; i < _shaders.size(); ++i) {
				if (bufData._bindings[i] == ~0ul)
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
			if (samplerData._bindings[i] == ~0ul)
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

RenderPassVk::RenderPassVk(Device &device) 
	: RenderPass(device)
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	_passCmds = deviceVk->GraphicsQueue().AllocateCmdBuffer();
}

void RenderPassVk::ClearAttachments()
{
	_renderPass.reset();
	_framebuffer.reset();
	RenderPass::ClearAttachments();
}

int RenderPassVk::AddAttachment(ContentTreatment inputContent, ContentTreatment outputContent, glm::vec4 clearValue)
{
	_renderPass.reset();
	_framebuffer.reset();
	return RenderPass::AddAttachment(inputContent, outputContent, clearValue);
}

void RenderPassVk::SetAttachmentImage(int attachmentIndex, std::shared_ptr<Image> const &img)
{
	_framebuffer.reset();
	RenderPass::SetAttachmentImage(attachmentIndex, img);
}

void RenderPassVk::Prepare()
{ 
	DeviceVk *deviceVk = GetDevice<DeviceVk>();

	if (!_renderPass)
		InitRenderPass(); 
	if (!_framebuffer)
		InitFramebuffer();

	PrepareToRecordRenderCommands();
	RecordPassCommands();
}

void RenderPassVk::Execute(PassDependencyTracker &dependencies)
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();

	std::vector<vk::Semaphore> waitSemaphores;
	std::vector<vk::PipelineStageFlags> waitStageFlags;
	FillDependencySemaphores(dependencies, DependencyType::Input, waitSemaphores, &waitStageFlags);
	std::vector<vk::Semaphore> signalSemaphores;
	FillDependencySemaphores(dependencies, DependencyType::Output, signalSemaphores);

	std::array<vk::SubmitInfo, 1> submits;

	submits[0]
		.setWaitSemaphoreCount(static_cast<uint32_t>(waitSemaphores.size()))
		.setPWaitSemaphores(waitSemaphores.data())
		.setPWaitDstStageMask(waitStageFlags.data())
		.setCommandBufferCount(1)
		.setPCommandBuffers(&*_passCmds)
		.setSignalSemaphoreCount(static_cast<uint32_t>(signalSemaphores.size()))
		.setPSignalSemaphores(signalSemaphores.data());
	deviceVk->GraphicsQueue()._queue.submit(submits, nullptr);
}

void RenderPassVk::InitRenderPass()
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();

	std::vector<vk::AttachmentDescription> attachmentDescs;
	for (auto &attach : _attachments) {
		ImageVk *img = attach.GetImage<ImageVk>();
		ImageVk::StateInfo stateInfo = img->GetStateInfo(ResourceState::RenderWrite);
		vk::AttachmentDescription attachDesc;
		attachDesc
			.setFormat(img->GetVkFormat())
			.setLoadOp(GetLoadOpFromContent(attach._inputContent))
			.setStencilLoadOp(attachDesc.loadOp)
			.setStoreOp(GetStoreOpFromContent(attach._outputContent))
			.setStencilStoreOp(attachDesc.storeOp)
			.setInitialLayout(stateInfo._layout)
			.setFinalLayout(stateInfo._layout);

		attachmentDescs.push_back(std::move(attachDesc));
	}

	std::vector<vk::AttachmentReference> colorRefs;
	vk::AttachmentReference depthRef;
	vk::SubpassDescription subpassDesc;
	for (uint32_t i = 0; i < _attachments.size(); ++i) {
		auto &attach = _attachments[i];
		ImageVk *img = attach.GetImage<ImageVk>();
		ImageVk::StateInfo stateInfo = img->GetStateInfo(ResourceState::RenderWrite);
		if (img->GetUsage() == Image::Usage::DepthBuffer) {
			ASSERT(!subpassDesc.pDepthStencilAttachment);
			depthRef.attachment = i;
			depthRef.layout = stateInfo._layout;
			subpassDesc.setPDepthStencilAttachment(&depthRef);
		} else {
			colorRefs.emplace_back(i, stateInfo._layout);
		}
	}
	subpassDesc
		.setColorAttachmentCount(static_cast<uint32_t>(colorRefs.size()))
		.setPColorAttachments(colorRefs.data());

	std::array<vk::SubpassDependency, 2> subpassDeps;
	subpassDeps[0]
		.setSrcSubpass(VK_SUBPASS_EXTERNAL)
		.setDstSubpass(0)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

	subpassDeps[1]
		.setSrcSubpass(0)
		.setDstSubpass(VK_SUBPASS_EXTERNAL)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite)
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead);


	vk::RenderPassCreateInfo passInfo;
	passInfo
		.setAttachmentCount(static_cast<uint32_t>(attachmentDescs.size()))
		.setPAttachments(attachmentDescs.data())
		.setSubpassCount(1)
		.setPSubpasses(&subpassDesc)
		.setDependencyCount(static_cast<uint32_t>(subpassDeps.size()))
		.setPDependencies(subpassDeps.data());

	_renderPass.reset();
	_renderPass = deviceVk->_device->createRenderPassUnique(passInfo, deviceVk->AllocationCallbacks());
}

void RenderPassVk::InitFramebuffer()
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();

	std::vector<vk::ImageView> attachmentViews;
	for (auto &attach : _attachments) {
		ImageVk *imageVk = static_cast<ImageVk*>(attach._image.get());
		attachmentViews.push_back(*imageVk->_view);
	}

	glm::uvec2 size = GetRenderAreaSize();

	vk::FramebufferCreateInfo frameInfo;
	frameInfo
		.setRenderPass(*_renderPass)
		.setAttachmentCount(static_cast<uint32_t>(attachmentViews.size()))
		.setPAttachments(attachmentViews.data())
		.setWidth(size.x)
		.setHeight(size.y)
		.setLayers(1);

	_framebuffer.reset();
	_framebuffer = deviceVk->_device->createFramebufferUnique(frameInfo, deviceVk->AllocationCallbacks());
}

void RenderPassVk::RecordPassCommands()
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();

	std::lock_guard<CmdBufferVk> passLock(_passCmds);

	_passCmds->reset(vk::CommandBufferResetFlags());

	vk::CommandBufferBeginInfo beginInfo;
	_passCmds->begin(beginInfo);

	std::vector<vk::ClearValue> clearValues;
	for (auto &attach : _attachments) {
		if (attach._image->GetUsage() == Image::Usage::DepthBuffer) {
			clearValues.emplace_back(vk::ClearDepthStencilValue(attach._clearValue[0], static_cast<uint32_t>(attach._clearValue[1])));
		} else {
			std::array<float, 4> color{ attach._clearValue[0], attach._clearValue[1], attach._clearValue[2], attach._clearValue[3] };
			clearValues.emplace_back(vk::ClearColorValue(color));
		}
	}

	glm::uvec2 size = GetRenderAreaSize();
	vk::RenderPassBeginInfo passBegin;
	passBegin
		.setRenderPass(*_renderPass)
		.setFramebuffer(*_framebuffer)
		.setRenderArea(vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(size.x, size.y)))
		.setClearValueCount(static_cast<uint32_t>(clearValues.size()))
		.setPClearValues(clearValues.data());

	_passCmds->beginRenderPass(passBegin, vk::SubpassContents::eSecondaryCommandBuffers);

	std::vector<vk::CommandBuffer> secondaryCmds;
	RecordRenderCommands(secondaryCmds);
	_passCmds->executeCommands(secondaryCmds);

	_passCmds->endRenderPass();

	_passCmds->end();
}

void RenderPassVk::PrepareToRecordRenderCommands()
{
	CommandPrepareInfoVk prepInfo(this);
	prepInfo._cmdInheritInfo
		.setRenderPass(*_renderPass)
		.setSubpass(GetSubpass());
		//.setFramebuffer(*_framebuffer);

	glm::uvec2 size = GetRenderAreaSize();
	prepInfo._viewport
		.setX(0)
		.setY(0)
		.setWidth(static_cast<float>(size.x))
		.setHeight(static_cast<float>(size.y))
		.setMinDepth(0.0f)
		.setMaxDepth(1.0f);

	std::for_each(std::execution::par_unseq, _commands.begin(), _commands.end(), [&](auto &&renderCmd) {
		renderCmd->PrepareToRecord(prepInfo);
	});
}

void RenderPassVk::RecordRenderCommands(std::vector<vk::CommandBuffer> &secondaryCmds)
{
	CommandRecordInfoVk recInfo;
	recInfo._secondaryCmds = &secondaryCmds;

	for (auto &renderCmd : _commands) {
		renderCmd->Record(recInfo);
	}
}


vk::AttachmentLoadOp RenderPassVk::GetLoadOpFromContent(ContentTreatment content)
{
	switch (content) {
		case ContentTreatment::Keep:
			return vk::AttachmentLoadOp::eLoad;
		case ContentTreatment::Clear:
			return vk::AttachmentLoadOp::eClear;
		case ContentTreatment::DontCare:
			return vk::AttachmentLoadOp::eDontCare;
	}
	throw GraphicsException("Invalid value for ContentTreatment!", VK_RESULT_MAX_ENUM);
}

vk::AttachmentStoreOp RenderPassVk::GetStoreOpFromContent(ContentTreatment content)
{
	switch (content) {
		case ContentTreatment::Keep:
			return vk::AttachmentStoreOp::eStore;
		case ContentTreatment::DontCare:
			return vk::AttachmentStoreOp::eDontCare;
	}
	throw GraphicsException("Invalid value for ContentTreatment!", VK_RESULT_MAX_ENUM);
}

NAMESPACE_END(gr1)

