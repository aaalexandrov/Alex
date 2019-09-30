#pragma once

#include "graphics_resource.h"
#include "material.h"
#include "buffer.h"

NAMESPACE_BEGIN(gr)

enum class PrimitiveTopology {
  TriangleList,
  TriangleStrip,
};

class Model : public GraphicsResource {
public:
  
  Material *GetMaterial() const { return _material.get(); }
  void SetMaterial(Material *material);

  int GetBufferCount() const { return static_cast<int>(_buffers.size()); }
  Buffer *GetBuffer(int index) const { return _buffers[index].get(); }
  void AddBuffer(Buffer *buffer);
  void RemoveBuffer(Buffer *buffer);

  virtual void Invalidate() {}

  std::shared_ptr<Material> _material;
  std::vector<std::shared_ptr<Buffer>> _buffers;
  PrimitiveTopology _topology = PrimitiveTopology::TriangleList;
};

NAMESPACE_END(gr)

RTTI_BIND(gr::Model, gr::GraphicsResource)