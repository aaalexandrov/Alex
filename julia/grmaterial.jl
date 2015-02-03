type UniformBlockBuffer
	block::UniformBlock
	buffer::Vector{Uint8}

	# todo: add the option to have own GL buffer object
	# todo: add info for dirty region

	UniformBlockBuffer(block::UniformBlock) = new(block, zeros(Uint8, block.size))
end

type Material
	shader::Shader
	uniforms::Dict{Symbol, Any}
	blockBuffers::Vector{UniformBlockBuffer}
	states::RenderStateHolder

	Material(shader::Shader) = new(shader, Dict{Symbol, Any}(), Array(UniformBlockBuffer, length(shader.blocks)), RenderStateHolder())
end

isvalid(mat::Material) = isvalid(mat.shader)


set_world_transform(mat::Material, m::Matrix) = setuniform(mat, mat.shader.worldTransform, m)
get_world_transform(mat::Material) = getuniform(mat, mat.shader.worldTransform)
has_world_transform(mat::Material) = hasuniform(mat, mat.shader.worldTransform)

function set_camera_transforms(mat::Material, view::Matrix, proj::Matrix)
	# these will not be set to a material that doesn't have them already
	# this way these transforms can use a separate material that can be updated and applied to the shader once per frame
	setuniform(mat, mat.shader.viewTransform, view, allowAdd = false)
	setuniform(mat, mat.shader.projTransform, proj, allowAdd = false)
end

set_camera_transforms(mat::Material, cam::Camera) = set_camera_transforms(mat, getview(cam), getproj(cam))
set_camera_transforms(mat::Material, ::Nothing) = nothing
getcamera(::Nothing) = nothing

function apply(mat::Material, renderer::Renderer)
	apply(mat.shader)

	set_camera_transforms(mat, getcamera(renderer))

	for (u, v) in mat.uniforms
		setuniform(mat.shader, u, v)
	end

	for i in 1:length(mat.blockBuffers)
		if isdefined(mat.blockBuffers, i)
			buffer = mat.blockBuffers[i]
			setbufferdata(buffer.block, buffer.buffer)
		end
	end

	apply(mat.states, renderer)
end

setstate(mat::Material, state::RenderState) = setstate(mat.states, state)
getstate{T <: RenderState}(mat::Material, ::Type{T}, default) = getstate(mat.states, T, default)

function setuniform(mat::Material, uniform::Symbol, value; allowAdd = true)
	if haskey(mat.shader.uniforms, uniform)
		var = mat.shader.uniforms[uniform]
		if inblock(var)
			if !isdefined(mat.blockBuffers, int(var.blockId))
				if !allowAdd
					return false
				end
				mat.blockBuffers[var.blockId] = UniformBlockBuffer(mat.shader.blocks[var.blockId])
			end
			setvalue(var, mat.blockBuffers[var.blockId].buffer, value)
		else
			if !allowAdd && !haskey(mat.uniforms, uniform)
				return false
			end
			mat.uniforms[uniform] = value
		end
		return true
	end

	info("setuniform(Material): attempt to set inexistent uniform $uniform")
	return false
end

function getuniform(mat::Material, uniform::Symbol)
	if haskey(mat.shader.uniforms, uniform)
		var = mat.shader.uniforms[uniform]
		if inblock(var)
			if isdefined(mat.blockBuffers, int(var.blockId))
				return getvalue(var, mat.blockBuffers[var.blockId].buffer)
			end
		else
			return get(mat.uniforms, uniform, nothing)
		end
	end
	return nothing
end

function hasuniform(mat::Material, uniform::Symbol)
	if !haskey(mat.shader.uniforms, uniform)
		return false
	end
	var = mat.shader.uniforms[uniform]
	if inblock(var) && isdefined(mat.blockBuffers, int(var.blockId))
		return true
	end
	return haskey(mat.uniforms, uniform)
end
