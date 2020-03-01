#pragma once

#include "gr1/shader.h"
#include "gr1/buffer.h"
#include "gr1/image.h"
#include "gr1/render_state.h"
#include "gr1/render_pipeline.h"
#include "gr1/execution_queue.h"
#include "gr1/render_commands.h"

NAMESPACE_BEGIN(gf)

class Material : public std::enable_shared_from_this<Material> {
public:

public:
	gr1::ShaderKindsArray<std::shared_ptr<gr1::Shader>> _shaders;
	gr1::BufferData _perMaterialUniforms;
	std::unordered_map<util::StrId, gr1::SamplerData> _textures;
};

NAMESPACE_END(gf)