#include "model.h"

NAMESPACE_BEGIN(gr)

void Model::SetMaterial(Material *material)
{
  _material = material->SharedFromType<Material>();
  Invalidate();
}

void Model::AddBuffer(Buffer *buffer)
{
  ASSERT(std::find_if(_buffers.begin(), _buffers.end(), [=](auto buf) { return buf.get() == buffer; }) != _buffers.end());
  _buffers.push_back(buffer->SharedFromType<Buffer>());
  Invalidate();
}

void Model::RemoveBuffer(Buffer *buffer)
{
  auto it = std::find_if(_buffers.begin(), _buffers.end(), [=](auto buf) { return buf.get() == buffer; });
  ASSERT(it != _buffers.end());
  _buffers.erase(it);
  Invalidate();
}

NAMESPACE_END(gr)

