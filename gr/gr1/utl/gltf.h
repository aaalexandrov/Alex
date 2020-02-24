#pragma once

#include "../buffer.h"
#include "../image.h"
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

	struct MaterialData {
		std::unordered_map<std::string, std::shared_ptr<Image>> _textures;
	};

	PrimitiveKind _primitiveKind;
	std::vector<BufferData> _buffers;
	MaterialData _material;
	std::unordered_map<std::string, std::string> _remapNames;
};

std::shared_ptr<Model> LoadGltfModel(Device &device, tinygltf::Model &gltfModel, std::unordered_map<std::string, std::string> remapNames = {});

NAMESPACE_END(gr1)