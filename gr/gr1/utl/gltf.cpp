#include "gltf.h"
#include "../device.h"
#include "../buffer.h"
#include "../output_pass.h"
#include "../execution_queue.h"
#include "../render_pass.h"
#include "util/layout.h"
#include "util/utl.h"

NAMESPACE_BEGIN(gr1)

uint32_t Model::GetIndicesCount()
{
	return static_cast<uint32_t>(GetIndexBuffer()->GetBufferLayout()->GetArrayCount());
}

Buffer *Model::GetIndexBuffer()
{
	auto it = std::find_if(_buffers.begin(), _buffers.end(), [&](auto const &buf) { return !!(buf._buffer->GetUsage() & Buffer::Usage::Index); });
	if (it == _buffers.end())
		return nullptr;
	return it->_buffer.get();
}

void Model::SetToDrawCommand(std::shared_ptr<RenderDrawCommand> const &drawCmd)
{
	int binding = 0;
	for (auto &buffer : _buffers) {
		if (!!(buffer._buffer->GetUsage() & Buffer::Usage::Index)) {
			drawCmd->AddBuffer(buffer._buffer);
			continue;
		}
		ASSERT(buffer._buffer->GetUsage() & Buffer::Usage::Vertex);
		if (!drawCmd->GetShader(ShaderKind::Vertex)->HasCommonVertexAttributes(buffer._buffer->GetBufferLayout()->GetArrayElement()))
			continue;
		drawCmd->AddBuffer(buffer._buffer, binding++, 0, buffer._perInstance);
	}
	drawCmd->SetDrawCounts(GetIndicesCount());
}

std::unordered_map<std::pair<uint32_t, uint32_t>, rttr::type> s_typeWithComponent2type{ {
	{{ TINYGLTF_TYPE_VEC2, TINYGLTF_COMPONENT_TYPE_FLOAT }, rttr::type::get<glm::vec2>() },
	{{ TINYGLTF_TYPE_VEC3, TINYGLTF_COMPONENT_TYPE_FLOAT }, rttr::type::get<glm::vec3>() },
	{{ TINYGLTF_TYPE_VEC4, TINYGLTF_COMPONENT_TYPE_FLOAT }, rttr::type::get<glm::vec4>() },

	{{ TINYGLTF_TYPE_SCALAR, TINYGLTF_COMPONENT_TYPE_BYTE }, rttr::type::get<int8_t>() },
	{{ TINYGLTF_TYPE_SCALAR, TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE }, rttr::type::get<uint8_t>() },
	{{ TINYGLTF_TYPE_SCALAR, TINYGLTF_COMPONENT_TYPE_SHORT }, rttr::type::get<int16_t>() },
	{{ TINYGLTF_TYPE_SCALAR, TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT }, rttr::type::get<uint16_t>() },
	{{ TINYGLTF_TYPE_SCALAR, TINYGLTF_COMPONENT_TYPE_INT }, rttr::type::get<int32_t>() },
	{{ TINYGLTF_TYPE_SCALAR, TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT }, rttr::type::get<uint32_t>() },
	{{ TINYGLTF_TYPE_SCALAR, TINYGLTF_COMPONENT_TYPE_FLOAT }, rttr::type::get<float>() },
	{{ TINYGLTF_TYPE_SCALAR, TINYGLTF_COMPONENT_TYPE_DOUBLE }, rttr::type::get<double>() },
}};

std::unordered_map<uint32_t, PrimitiveKind> s_gltf2primitiveKind{ {
	{ TINYGLTF_MODE_TRIANGLES, PrimitiveKind::TriangleList },
	{ TINYGLTF_MODE_TRIANGLE_STRIP, PrimitiveKind::TriangleStrip },
} };

void AddModelBuffer(Device &device, Model &model, std::vector<uint8_t> const &buffer, Buffer::Usage usage, std::shared_ptr<util::LayoutElement> const &layout, size_t offset)
{
	auto staging = device.CreateResource<Buffer>();
	staging->Init(Buffer::Usage::Staging, layout);
	size_t size = layout->GetSize();
	void *mapped = staging->Map();
	memcpy(mapped, buffer.data() + offset, size);
	staging->Unmap();

	model._buffers.push_back({ device.CreateResource<Buffer>() });
	model._buffers.back()._buffer->Init(usage, layout);

	auto copyPass = device.CreateResource<BufferCopyPass>();
	copyPass->Init(staging, model._buffers.back()._buffer);

	device.GetExecutionQueue().EnqueuePass(copyPass);
}

std::shared_ptr<Model> LoadGltfModel(Device &device, tinygltf::Model &gltfModel, std::unordered_map<std::string, std::string> const &remapAttributes)
{
	ASSERT(gltfModel.buffers.size() == 1);
	std::shared_ptr<util::LayoutStruct> bufLayout = std::make_shared<util::LayoutStruct>();
	tinygltf::Mesh &mesh = gltfModel.meshes[0];
	auto &primitive = mesh.primitives[0];
	auto &indAccessor = gltfModel.accessors[primitive.indices];

	auto model = std::make_shared<Model>();
	model->_primitiveKind = s_gltf2primitiveKind.at(primitive.mode);

	for (int v = 0; v < gltfModel.bufferViews.size(); ++v) {
		auto &view = gltfModel.bufferViews[v];

		if (v == indAccessor.bufferView) {
			ASSERT(view.target == TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER);
			auto value = util::CreateLayoutValue(s_typeWithComponent2type.at(std::pair(indAccessor.type, indAccessor.componentType)));
			auto indArray = util::CreateLayoutArray(value, indAccessor.count);
			bufLayout->AddField("_indices", indArray, indAccessor.byteOffset);
			continue;
		} 

		std::string name = view.name.size() ? view.name : (std::string("_vertexStream") + std::to_string(v));
		size_t maxElements = ~0ull;
		auto elem = std::make_shared<util::LayoutStruct>();
		elem->SetPadding(view.byteStride);
		for (auto attr : primitive.attributes) {
			auto &accessor = gltfModel.accessors[attr.second];
			if (accessor.bufferView != v)
				continue;
			ASSERT(view.target == TINYGLTF_TARGET_ARRAY_BUFFER);
			maxElements = std::min(maxElements, accessor.count);
			auto value = util::CreateLayoutValue(s_typeWithComponent2type.at(std::pair(accessor.type, accessor.componentType)));
			std::string name = util::FindOrDefault(remapAttributes, attr.first, attr.first);
			elem->AddField(name, value, accessor.byteOffset);
		}

		ASSERT(!view.byteStride || maxElements <= (view.byteLength - view.byteOffset) / view.byteStride);
		if (elem->GetStructFieldCount()) {
			auto elemArray = util::CreateLayoutArray(elem, maxElements);
			bufLayout->AddField(name, elemArray, view.byteOffset);
		}
	}

	auto &buffer = gltfModel.buffers[0];
	ASSERT(buffer.data.size() >= bufLayout->GetSize());

	for (size_t i = 0; i < bufLayout->GetStructFieldCount(); ++i) {
		std::string name = bufLayout->GetStructFieldName(i);
		util::LayoutElement *elem = const_cast<util::LayoutElement *>(bufLayout->GetStructFieldElement(i));
		size_t offset = bufLayout->GetStructFieldOffset(i);

		Buffer::Usage usage = name == "_indices" ? Buffer::Usage::Index : Buffer::Usage::Vertex;
		AddModelBuffer(device, *model, buffer.data, usage, elem->shared_from_this(), offset);
	}

	return model;
}

NAMESPACE_END(gr1)
