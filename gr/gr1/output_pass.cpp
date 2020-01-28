#include "output_pass.h"
#include "buffer.h"
#include "presentation_surface.h"

NAMESPACE_BEGIN(gr1)

void ResourceStateTransitionPass::Init(Resource &resource, ResourceState srcState, ResourceState dstState)
{
	_resource = &resource;
	_srcState = srcState;
	_dstState = dstState;
}


void CopyBufferPass::Init(std::shared_ptr<Buffer> const &srcBuffer, std::shared_ptr<Buffer> const &dstBuffer, uint32_t offset, uint32_t size)
{
	_src = srcBuffer;
	_dst = dstBuffer;
	_offset = offset;
	_size = _size;

	ASSERT(_src->GetSize() >= _offset + _size);
	ASSERT(_dst->GetSize() >= _offset + _size);
}

void CopyBufferPass::GetDependencies(DependencyType dependencyType, DependencyFunc addDependencyFunc)
{
	if (dependencyType == DependencyType::Input) {
		addDependencyFunc(_src.get(), ResourceState::TransferRead);
		addDependencyFunc(_dst.get(), ResourceState::TransferWrite);
	}
}


void PresentPass::Init(std::shared_ptr<PresentationSurface> const &presentSurface)
{
	_surface = presentSurface;
}

void PresentPass::GetDependencies(DependencyType dependencyType, DependencyFunc addDependencyFunc)
{
	if (dependencyType == DependencyType::Input) {
		addDependencyFunc(_surface.get(), ResourceState::PresentRead);
	}
}

NAMESPACE_END(gr1)

