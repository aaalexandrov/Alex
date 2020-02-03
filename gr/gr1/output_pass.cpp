#include "output_pass.h"
#include "buffer.h"
#include "image.h"
#include "presentation_surface.h"

NAMESPACE_BEGIN(gr1)

void ResourceStateTransitionPass::Init(Resource &resource, ResourceState srcState, ResourceState dstState)
{
	_resource = &resource;
	_srcState = srcState;
	_dstState = dstState;
}


void BufferCopyPass::Init(std::shared_ptr<Buffer> const &srcBuffer, std::shared_ptr<Buffer> const &dstBuffer, uint32_t srcOffset, uint32_t dstOffset, uint32_t size)
{
	_src = srcBuffer;
	_dst = dstBuffer;
	_srcOffset = srcOffset;
	_dstOffset = dstOffset;
	_size = _size;

	ASSERT(_src->GetSize() >= _srcOffset + _size);
	ASSERT(_dst->GetSize() >= _dstOffset + _size);
}

void BufferCopyPass::GetDependencies(DependencyType dependencyType, DependencyFunc addDependencyFunc)
{
	if (dependencyType == DependencyType::Input) {
		addDependencyFunc(_src.get(), !(_src->GetUsage() & Buffer::Usage::Staging) ? ResourceState::TransferRead : ResourceState::Initial);
		addDependencyFunc(_dst.get(), !(_dst->GetUsage() & Buffer::Usage::Staging) ? ResourceState::TransferWrite : ResourceState::Initial);
	}
}


void PresentPass::Init(std::shared_ptr<PresentationSurface> const &presentSurface)
{
	_surface = presentSurface;
}

void PresentPass::SetImageToPresent(std::shared_ptr<Image> const &surfaceImage)
{
	_surfaceImage = surfaceImage;
}

void PresentPass::GetDependencies(DependencyType dependencyType, DependencyFunc addDependencyFunc)
{
	if (dependencyType == DependencyType::Input) {
		addDependencyFunc(_surfaceImage.get(), ResourceState::PresentRead);
	}
}

NAMESPACE_END(gr1)

