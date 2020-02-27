#include "render_pass_vk.h"
#include "device_vk.h"
#include "image_vk.h"
#include "../graphics_exception.h"
#include "rttr/registration.h"

NAMESPACE_BEGIN(gr1)

RTTR_REGISTRATION
{
	using namespace rttr;
	registration::class_<RenderPassVk>("RenderPassVk")(metadata(RttrDiscriminator::Tag, type::get<DeviceVk>()))
		.constructor<Device&>()(policy::ctor::as_raw_ptr);
}

CommandPrepareInfoVk::CommandPrepareInfoVk(RenderPassVk *renderPassVk)
	: CommandPrepareInfo(renderPassVk)
{
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

