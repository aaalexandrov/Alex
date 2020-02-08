#include "render_pass_vk.h"
#include "device_vk.h"
#include "image_vk.h"
#include "shader_vk.h"
#include "buffer_vk.h"
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

RenderDrawCommandVk::RenderDrawCommandVk(Device &device) 
	: RenderDrawCommand(device) 
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();
	
	_cmdDraw = deviceVk->GraphicsQueue().AllocateCmdBuffer(vk::CommandBufferLevel::eSecondary);
}

void RenderDrawCommandVk::PrepareToRecord(CommandPrepareInfo &prepareInfo)
{
	CommandPrepareInfoVk *prepInfoVk = static_cast<CommandPrepareInfoVk*>(&prepareInfo);
	PreparePipelineLayout();
	PreparePipeline(prepInfoVk->_cmdInheritInfo.renderPass, prepInfoVk->_cmdInheritInfo.subpass);
	PrepareDescriptorSets();

	UpdateDescriptorSets();

	_cmdDraw->reset(vk::CommandBufferResetFlags());

	vk::CommandBufferBeginInfo beginInfo;
	beginInfo
		.setFlags(vk::CommandBufferUsageFlagBits::eRenderPassContinue)
		.setPInheritanceInfo(&prepInfoVk->_cmdInheritInfo);
	_cmdDraw->begin(beginInfo);

	_cmdDraw->bindPipeline(vk::PipelineBindPoint::eGraphics, *_pipeline);

	SetDynamicState(*prepInfoVk);

	std::vector<vk::DescriptorSet> descriptorSets;
	for (auto &desc : _descriptorSets) {
		if (!desc)
			continue;
		descriptorSets.push_back(*desc);
	}
	_cmdDraw->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *_pipelineLayout, 0, descriptorSets, nullptr);

	vk::Buffer indexBuffer;
	vk::IndexType indexType;
	size_t indexOffset;
	std::vector<vk::Buffer> vertBuffers;
	std::vector<size_t> vertBufferOffsets;
	for (auto &bufData : _buffers) {
		BufferVk *bufferVk = static_cast<BufferVk*>(bufData._buffer.get());
		if (!!(bufData._buffer->GetUsage() & Buffer::Usage::Vertex)) {
			vertBuffers.resize(std::max<size_t>(vertBuffers.size(), bufData._binding + 1));
			ASSERT(!vertBuffers[bufData._binding]);
			vertBuffers[bufData._binding] = *bufferVk->_buffer;
			vertBufferOffsets.resize(vertBuffers.size());
			vertBufferOffsets[bufData._binding] = bufData._offset;
		} else if (!!(bufData._buffer->GetUsage() & Buffer::Usage::Index)) {
			ASSERT(!indexBuffer);
			indexBuffer = *bufferVk->_buffer;
			indexType = bufferVk->GetVkIndexType();
			indexOffset = bufData._offset;
		}
	}
	_cmdDraw->bindVertexBuffers(0, vertBuffers, vertBufferOffsets);
	if (indexBuffer) {
		_cmdDraw->bindIndexBuffer(indexBuffer, indexOffset, indexType);

		_cmdDraw->drawIndexed(_indexCount, _instanceCount, _firstIndex, _vertexOffset, _instanceCount);
	} else {
		_cmdDraw->draw(_indexCount, _instanceCount, _firstIndex, _firstInstance);
	}

	_cmdDraw->end();
}

void RenderDrawCommandVk::Record(CommandRecordInfo &recordInfo)
{
	CommandRecordInfoVk *recInfoVk = static_cast<CommandRecordInfoVk*>(&recordInfo);

	recInfoVk->_secondaryCmds->push_back(_cmdDraw.get());
}

void RenderDrawCommandVk::PreparePipelineLayout()
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();

	std::vector<vk::DescriptorSetLayout> setLayouts;
	for (auto &shader : _shaders) {
		auto shaderVk = std::static_pointer_cast<ShaderVk>(shader);
		setLayouts.push_back(shaderVk->GetDescriptorSetLayout());
	}

	vk::PipelineLayoutCreateInfo pipelineInfo;
	pipelineInfo
		.setSetLayoutCount(static_cast<uint32_t>(setLayouts.size()))
		.setPSetLayouts(setLayouts.data());
	_pipelineLayout = deviceVk->_device->createPipelineLayoutUnique(pipelineInfo, deviceVk->AllocationCallbacks());
}

void RenderDrawCommandVk::PreparePipeline(vk::RenderPass renderPass, uint32_t subpass)
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();

	PipelineStore::PipelineInfo pipeInfo;
	for (int i = 0; i < _shaders.size(); ++i) {
		if (!_shaders[i])
			continue;
		pipeInfo._shaders.push_back(_shaders[i].get());
	}

	for (int i = 0; i < _buffers.size(); ++i) {
		if (!(_buffers[i]._buffer->GetUsage() & Buffer::Usage::Vertex))
			continue;

		PipelineStore::VertexBufferLayout bufLayout;
		bufLayout._bufferLayout = _buffers[i]._buffer->GetBufferLayout().get();
		bufLayout._binding = _buffers[i]._binding;
		bufLayout._frequencyInstance = _buffers[i]._frequencyInstance;
		pipeInfo._vertexBufferLayouts.push_back(bufLayout);
	}

	pipeInfo._renderState = _renderState.get();
	pipeInfo._primitiveKind = _primitiveKind;
	pipeInfo._pipelineLayout = *_pipelineLayout;
	pipeInfo._renderPass = renderPass;
	pipeInfo._subpass = subpass;
	
	_pipeline = deviceVk->_pipelineStore.CreatePipeline(pipeInfo);
}

void RenderDrawCommandVk::PrepareDescriptorSets()
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();

	for (int i = 0; i < _shaders.size(); ++i) {
		auto shaderVk = static_cast<ShaderVk*>(_shaders[i].get());
		_descriptorSets[i] = std::move(shaderVk ? shaderVk->AllocateDescriptorSet() : vk::UniqueDescriptorSet());
	}
}

void RenderDrawCommandVk::UpdateDescriptorSets()
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();

	std::vector<vk::DescriptorBufferInfo> bufferInfos;
	// reserve enough elements so that the array won't be reallocated, we're storing addresses of elements below
	bufferInfos.reserve(_buffers.size() * static_cast<uint32_t>(ShaderKind::Count));
	std::vector<vk::WriteDescriptorSet> setWrites;
	for (auto &bufData : _buffers) {
		if (!!(bufData._buffer->GetUsage() & Buffer::Usage::Uniform)) {
			for (uint32_t k = 0; k < static_cast<uint32_t>(ShaderKind::Count); ++k) {
				if (!(bufData._shaderKinds & static_cast<ShaderKindBits>(1 << k)))
					continue;

				BufferVk *bufferVk = static_cast<BufferVk*>(bufData._buffer.get());
				bufferInfos.emplace_back();
				bufferInfos.back()
					.setBuffer(*bufferVk->_buffer)
					.setOffset(0)
					.setRange(bufferVk->GetSize());

				setWrites.emplace_back();
				setWrites.back()
					.setDstSet(*_descriptorSets[k])
					.setDstBinding(bufData._binding)
					.setDstArrayElement(0)
					.setDescriptorCount(1)
					.setDescriptorType(vk::DescriptorType::eUniformBuffer)
					.setPBufferInfo(&bufferInfos.back());
			}
		}
	}
	
	deviceVk->_device->updateDescriptorSets(setWrites, nullptr);
}

void RenderDrawCommandVk::SetDynamicState(CommandPrepareInfoVk &prepareInfo)
{
	RenderStateVk *renderStateVk = static_cast<RenderStateVk*>(_renderState.get());
	std::vector<vk::Viewport> viewports;

	RenderState::Viewport const &viewport = renderStateVk->GetViewport();
	if (!viewport._rect.IsEmpty()) {
		renderStateVk->FillViewports(viewports);
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
	CommandPrepareInfoVk prepInfo;
	prepInfo._cmdInheritInfo
		.setRenderPass(*_renderPass)
		.setSubpass(GetSubpass())
		.setFramebuffer(*_framebuffer);

	glm::uvec2 size = GetRenderAreaSize();
	prepInfo._viewport
		.setX(0)
		.setY(0)
		.setWidth(static_cast<float>(size.x))
		.setHeight(static_cast<float>(size.y))
		.setMinDepth(0.0f)
		.setMaxDepth(1.0f);

	for (auto &renderCmd : _commands) {
		renderCmd->PrepareToRecord(prepInfo);
	}
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

