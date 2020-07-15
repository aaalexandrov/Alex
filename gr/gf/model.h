#pragma once

#include "gr1/buffer.h"
#include "material.h"

NAMESPACE_BEGIN(gf)

class Model : public std::enable_shared_from_this<Model> {
	RTTR_ENABLE()
public:

public:
	struct MaterialSlice {
		util::IntervalI _indices;
		std::shared_ptr<Material> _material;
	};

	std::vector<gr1::Buffer> _geometryBuffers;
	std::vector<MaterialSlice> _materials;
	gr1::PrimitiveKind _primitiveKind = gr1::PrimitiveKind::TriangleList;
};

NAMESPACE_END(gf)