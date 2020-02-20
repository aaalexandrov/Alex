#pragma once

#include "../buffer.h"
#include "util/namespace.h"
#include "tinygltf/tiny_gltf.h"

NAMESPACE_BEGIN(gr1)

class RenderDrawCommand;
class Model {
public:
	uint32_t GetIndicesCount();
	Buffer *GetIndexBuffer();
	void SetToDrawCommand(std::shared_ptr<RenderDrawCommand> const &drawCmd);

	struct BufferData {
		std::shared_ptr<Buffer> _buffer;
		bool _perInstance = false;
	};

	PrimitiveKind _primitiveKind;
	std::vector<BufferData> _buffers;
};

std::shared_ptr<Model> LoadGltfModel(Device &device, tinygltf::Model &gltfModel, std::unordered_map<std::string, std::string> const &remapAttributes = std::unordered_map<std::string, std::string>());

NAMESPACE_END(gr1)