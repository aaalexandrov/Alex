#include "render_pipeline.h"
#include "graphics_exception.h"
#include "render_state.h"

NAMESPACE_BEGIN(gr1)

size_t VertexBufferLayout::GetHash() const
{
	size_t hash = _bufferLayout->GetArrayElement()->GetHash();
	hash = util::GetHash(_frequencyInstance, hash);
	return hash;
}

bool VertexBufferLayout::operator==(VertexBufferLayout const &other) const
{
	return *_bufferLayout->GetArrayElement() == *other._bufferLayout->GetArrayElement()
		&& _frequencyInstance == other._frequencyInstance;
}

PipelineResource::Kind RenderPipeline::ResourceInfo::GetPipelineResourceKind() const
{
	switch (_kind) {
		case Shader::Parameter::IndexBuffer:
		case Shader::Parameter::VertexBuffer:
		case Shader::Parameter::UniformBuffer:
			return PipelineResource::Buffer;
		case Shader::Parameter::Sampler:
			return PipelineResource::Sampler;
	}
	throw GraphicsException("Unrecognized shader parameter kind", ~0u);
}

void RenderPipeline::ResourceInfo::AddUniqueBinding(uint32_t binding)
{
	for (int i = 0; i < _binding.size(); ++i) {
		if (_binding[i] == binding)
			return;
		if (_binding[i] == ~0u) {
			_binding[i] = binding;
			return;
		}
	}
}

void RenderPipeline::InitParamShaders(ShaderKindsArray<std::shared_ptr<Shader>> const &shaders, PrimitiveKind primitiveKind)
{
	ASSERT(std::find_if(_shaders.begin(), _shaders.end(), [](auto &s) { return s; }) == _shaders.end());
	_shaders = shaders;
	_primitiveKind = primitiveKind;
}

void RenderPipeline::InitParamVertexBufferLayout(util::StrId bufferId, std::shared_ptr<util::LayoutElement> const &bufferLayout, bool frequencyInstance)
{
	ASSERT(bufferId);
	ASSERT(std::find_if(_vertexBufferLayouts.begin(), _vertexBufferLayouts.end(), [&](auto &l) { 
			return 
				l._bufferId == bufferId 
				|| Shader::HasCommonVertexAttributes(l._bufferLayout.get(), bufferLayout.get(), nullptr); 
		}) == _vertexBufferLayouts.end());

	ASSERT(_shaders[ShaderKind::Vertex]->HasCommonVertexAttributes(bufferLayout.get(), nullptr));

	VertexBufferLayout vbLayout;
	vbLayout._bufferLayout = bufferLayout;
	vbLayout._bufferId = bufferId;
	vbLayout._frequencyInstance = frequencyInstance;
	_vertexBufferLayouts.push_back(vbLayout);
}

void RenderPipeline::Init()
{
	InitResourceInfos();
}

void RenderPipeline::SetRenderState(std::shared_ptr<RenderState> const &renderState)
{
	_renderState = renderState;
	_renderStateDataVersion = renderState->GetStateDataVersion();
}

void RenderPipeline::SetRenderPass(std::weak_ptr<RenderPass> const &renderPass)
{
	_renderPass = renderPass;
}

void RenderPipeline::AddResourceInfo(ResourceInfo &info)
{
	uint32_t infoIndex = static_cast<uint32_t>(_resourceInfos.size());
	_resourceInfos.push_back(info);
	_id2Info.insert(std::make_pair(info._id, infoIndex));
}

void RenderPipeline::InitResourceInfos()
{
	ASSERT(std::find_if(_shaders.begin(), _shaders.end(), [](auto &s) { return s; }) != _shaders.end());
	ASSERT(!_resourceInfos.size() && !_id2Info.size());

	std::sort(_vertexBufferLayouts.begin(), _vertexBufferLayouts.end(), 
		[](auto const &l1, auto const &l2) { return *l1._bufferLayout < *l2._bufferLayout; });

	std::array<uint32_t, PipelineResource::Kind::Count> paramIndices{0, 0};
	ResourceInfo indicesInfo;
	indicesInfo._kind = Shader::Parameter::IndexBuffer;
	AddResourceInfo(indicesInfo);

	for (uint32_t i = 0; i < _vertexBufferLayouts.size(); ++i) {
		auto &vbLayout = _vertexBufferLayouts[i];
		ResourceInfo vbInfo;
		vbInfo._id = vbLayout._bufferId;
		vbInfo._kind = Shader::Parameter::VertexBuffer;
		vbInfo._indexInShader[ShaderKind::Vertex] = i;
		vbInfo._binding[ShaderKind::Vertex] = i;
		AddResourceInfo(vbInfo);
	}

	for (int i = 0; i < _shaders.size(); ++i) {
		for (int paramKind = 0; paramKind < Shader::Parameter::Count; ++paramKind) {
			auto const &params = _shaders[i]->GetParameters(static_cast<Shader::Parameter::Kind>(paramKind));
			for (uint32_t paramIndex = 0; paramIndex < params.size(); ++paramIndex) {
				auto const &param = params[paramIndex];
				auto itInfo = _id2Info.find(param._id);
				if (itInfo != _id2Info.end()) {
					ResourceInfo &parInfo = _resourceInfos[itInfo->second];
					ASSERT(parInfo._kind == paramKind);
					parInfo._indexInShader[i] = paramIndex;
					parInfo.AddUniqueBinding(param._binding);
					continue;
				}
				ResourceInfo parInfo;
				parInfo._id = param._id;
				parInfo._kind = static_cast<Shader::Parameter::Kind>(paramKind);
				parInfo._indexInShader[i] = paramIndex;
				parInfo._binding[i] = param._binding;
				AddResourceInfo(parInfo);
			}
		}
	}
}

NAMESPACE_END(gr1)

