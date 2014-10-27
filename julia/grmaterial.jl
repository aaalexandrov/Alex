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

function apply(mat::Material, renderer::Union(Renderer, Nothing) = nothing)
	apply(mat.shader)
	
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

setworldtransform(mat::Material, m::Matrix) = setuniform(mat, mat.shader.worldTransform, m)
getworldtransform(mat::Material) = getuniform(mat, mat.shader.worldTransform)

setstate(mat::Material, state::RenderState) = setstate(mat.states, state)
getstate{T <: RenderState}(mat::Material, ::Type{T}, default) = getstate(mat.states, T, default)

function setuniform(mat::Material, uniform::Symbol, value)
	if haskey(mat.shader.uniforms, uniform)
		var = mat.shader.uniforms[uniform]
		if inblock(var)
			if !isdefined(mat.blockBuffers, int(var.blockId))
				mat.blockBuffers[var.blockId] = UniformBlockBuffer(mat.shader.blocks[var.blockId])
			end
			setvalue(var, mat.blockBuffers[var.blockId].buffer, value)
		else
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
