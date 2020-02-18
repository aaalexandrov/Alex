#pragma once

#include "../buffer.h"
#include "util/namespace.h"
#include "tinygltf/tiny_gltf.h"

NAMESPACE_BEGIN(gr1)

class Model {
public:

	std::vector<Buffer> _buffers;
};

std::shared_ptr<Model> LoadGltfModel(Device &device, tinygltf::Model &gltfModel);

NAMESPACE_END(gr1)