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

void RenderPipeline::Init(ShaderKindsArray<std::shared_ptr<Shader>> const &shaders, std::vector<VertexBufferLayout> const &vertexBufferLayouts, PrimitiveKind primitiveKind)
{
	_shaders = shaders;
	_primitiveKind = primitiveKind;

	for (auto &vbLayout : vertexBufferLayouts) {
		AddVertexBufferLayout(vbLayout);
	}

	InitResourceInfos();
}

void RenderPipeline::SetRenderState(std::shared_ptr<RenderState> const &renderState)
{
	_renderState = renderState;
	_renderStateDataVersion = renderState->GetStateDataVersion();
	SetResourceState(ResourceState::Invalidated);
}

void RenderPipeline::SetRenderPass(std::weak_ptr<RenderPass> const &renderPass)
{
	_renderPass = renderPass;
	SetResourceState(ResourceState::Invalidated);
}

ResourceState RenderPipeline::UpdateResourceStateForExecute()
{
	if (_state == ResourceState::ShaderRead) {
		if (_renderState->GetResourceState() == ResourceState::Invalidated) {
			SetResourceState(ResourceState::Invalidated);
		}
	}
	return _state;
}

void RenderPipeline::AddVertexBufferLayout(VertexBufferLayout const &vbLayout)
{
	ASSERT(vbLayout._bufferId);
	ASSERT(std::find_if(_vertexBufferLayouts.begin(), _vertexBufferLayouts.end(), [&](auto &l) {
		return
			l._bufferId == vbLayout._bufferId
			|| Shader::HasCommonVertexAttributes(l._bufferLayout.get(), vbLayout._bufferLayout.get(), nullptr);
	}) == _vertexBufferLayouts.end());

	ASSERT(_shaders[ShaderKind::Vertex]->HasCommonVertexAttributes(vbLayout._bufferLayout.get(), nullptr));

	_vertexBufferLayouts.push_back(vbLayout);
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
	ASSERT(!_resourceInfos.size() && !_id2Info.size() && !_vertexBufferIndices.size());

	std::sort(_vertexBufferLayouts.begin(), _vertexBufferLayouts.end(), 
		[](auto const &l1, auto const &l2) { return *l1._bufferLayout < *l2._bufferLayout; });

	std::array<uint32_t, PipelineResource::Kind::Count> paramIndices{0, 0};
	ResourceInfo indicesInfo;
	indicesInfo._kind = Shader::Parameter::IndexBuffer;
	_indexBufferIndex = static_cast<int32_t>(_resourceInfos.size());
	AddResourceInfo(indicesInfo);

	for (uint32_t i = 0; i < _vertexBufferLayouts.size(); ++i) {
		auto &vbLayout = _vertexBufferLayouts[i];
		ResourceInfo vbInfo;
		vbInfo._id = vbLayout._bufferId;
		vbInfo._kind = Shader::Parameter::VertexBuffer;
		vbInfo._indexInShader[ShaderKind::Vertex] = i;
		vbInfo._binding[ShaderKind::Vertex] = i;
		_vertexBufferIndices.push_back(static_cast<int32_t>(_resourceInfos.size()));
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

PipelineResource::Kind RenderPipeline::GetPipelineResourceKind(Shader::Parameter::Kind kind)
{
	switch (kind) {
		case Shader::Parameter::IndexBuffer:
		case Shader::Parameter::VertexBuffer:
		case Shader::Parameter::UniformBuffer:
			return PipelineResource::Buffer;
		case Shader::Parameter::Sampler:
			return PipelineResource::Sampler;
	}
	throw GraphicsException("Unrecognized shader parameter kind", ~0u);
}

NAMESPACE_END(gr1)

