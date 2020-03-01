#pragma once

#include "gr1/buffer.h"
#include "material.h"

NAMESPACE_BEGIN(gf)

class Model : public std::enable_shared_from_this<Model> {
public:

public:
	std::shared_ptr<Material> _material;
	gr1::PrimitiveKind _primitiveKind = gr1::PrimitiveKind::TriangleList;
	std::unordered_map<util::StrId, gr1::BufferData> _geometryBuffers;
	std::shared_ptr<gr1::RenderPipeline> _pipeline;
};

NAMESPACE_END(gf)