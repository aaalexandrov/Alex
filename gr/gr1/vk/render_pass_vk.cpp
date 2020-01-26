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

void RenderPassVk::InitRenderPass()
{
	DeviceVk *deviceVk = GetDevice<DeviceVk>();

	std::vector<vk::AttachmentDescription> attachmentDescs;
	for (auto &attach : _attachments) {
		ImageVk *img = attach.GetImage<ImageVk>();
		vk::AttachmentDescription attachDesc;
		attachDesc
			.setFormat(img->GetVkFormat())
			.setLoadOp(GetLoadOpFromContent(attach._inputContent))
			.setStencilLoadOp(attachDesc.loadOp)
			.setStoreOp(GetStoreOpFromContent(attach._outputContent))
			.setStencilStoreOp(attachDesc.storeOp)
			.setInitialLayout(img->_layout)
			.setFinalLayout(img->_layout);

		attachmentDescs.push_back(std::move(attachDesc));
	}

	std::vector<vk::AttachmentReference> colorRefs;
	vk::AttachmentReference depthRef;
	vk::SubpassDescription subpassDesc;
	for (uint32_t i = 0; i < _attachments.size(); ++i) {
		auto &attach = _attachments[i];
		ImageVk *img = attach.GetImage<ImageVk>();
		if (img->GetUsage() == Image::Usage::DepthBuffer) {
			ASSERT(!subpassDesc.pDepthStencilAttachment);
			depthRef.attachment = i;
			depthRef.layout = img->_layout;
			subpassDesc.setPDepthStencilAttachment(&depthRef);
		} else {
			colorRefs.emplace_back(i, img->_layout);
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
		.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

	subpassDeps[1]
		.setSrcSubpass(0)
		.setDstSubpass(VK_SUBPASS_EXTERNAL)
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

	_renderPass = deviceVk->_device->createRenderPassUnique(passInfo, deviceVk->AllocationCallbacks());
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

