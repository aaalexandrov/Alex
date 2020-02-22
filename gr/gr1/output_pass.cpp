#include "output_pass.h"
#include "buffer.h"
#include "image.h"
#include "presentation_surface.h"

NAMESPACE_BEGIN(gr1)

void ResourceStateTransitionPass::Init(std::shared_ptr<Resource> const &resource, ResourceState srcState, ResourceState dstState)
{
	_resource = resource;
	_srcState = srcState;
	_dstState = dstState;
}


void BufferCopyPass::Init(std::shared_ptr<Buffer> const &srcBuffer, std::shared_ptr<Buffer> const &dstBuffer, uint32_t size, uint32_t srcOffset, uint32_t dstOffset)
{
	_src = srcBuffer;
	_dst = dstBuffer;
	_srcOffset = srcOffset;
	_dstOffset = dstOffset;
	_size = std::min(size, std::min(srcBuffer->GetSize() - _srcOffset, dstBuffer->GetSize() - _dstOffset));

	ASSERT(_src->GetSize() >= _srcOffset + _size);
	ASSERT(_dst->GetSize() >= _dstOffset + _size);
}

void BufferCopyPass::GetDependencies(DependencyType dependencyType, DependencyFunc addDependencyFunc)
{
	if (dependencyType == DependencyType::Input) {
		addDependencyFunc(_src.get(), ResourceState::TransferRead);
		addDependencyFunc(_dst.get(), ResourceState::TransferWrite);
	}
}


void ImageBufferCopyPass::Init(
	std::shared_ptr<Buffer> const &buffer,
	std::shared_ptr<Image> const &image,
	CopyDirection direction,
	glm::uvec4 bufferOffset,
	glm::uvec4 bufferSize,
	glm::uvec4 imageOffset,
	glm::uvec4 size,
	uint32_t imageMipLevel)
{
	_direction = direction;
	_buffer = buffer;
	_image = image;
	if (size == glm::zero<glm::uvec4>())
		size = _image->GetSize();
	if (bufferSize == glm::zero<glm::uvec4>()) 
		bufferSize = _image->GetSize();
	_bufferImageData._size = bufferSize;
	_bufferImageData._pitch = ImageData::GetPackedPitch(bufferSize, _image->GetColorFormatSize());
	_bufferImageData._data = nullptr;
	_bufferOffset = bufferOffset;
	_imageOffset = imageOffset;
	_size = size;
	_imageMipLevel = imageMipLevel;

	ASSERT(_direction == CopyDirection::ImageToBuffer || _direction == CopyDirection::BufferToImage);
	ASSERT(glm::all(glm::lessThanEqual(_bufferOffset + _size, _bufferImageData._size)));
	ASSERT(glm::all(glm::lessThanEqual(_imageOffset + _size, _image->GetSize())));
	ASSERT(_imageMipLevel < _image->GetMipLevels());
}

void ImageBufferCopyPass::GetDependencies(DependencyType dependencyType, DependencyFunc addDependencyFunc)
{
	if (dependencyType == DependencyType::Input) {
		addDependencyFunc(_buffer.get(), GetBufferState());
		addDependencyFunc(_image.get(), GetImageState());
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

