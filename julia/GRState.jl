abstract RenderState
abstract AlphaBlendState <: RenderState
abstract StencilState <: RenderState
abstract DepthState <: RenderState
abstract CullState <: RenderState


type AlphaBlendDisabled <: AlphaBlendState
end

function setstate(::AlphaBlendDisabled)
	glDisable(GL_BLEND)
end

type AlphaBlendSrcAlpha <: AlphaBlendState
end

function setstate(::AlphaBlendSrcAlpha)
	glEnable(GL_BLEND)
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD)
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ZERO)
end

type AlphaBlendAdditive <: AlphaBlendState
end

function setstate(::AlphaBlendAdditive)
	glEnable(GL_BLEND)
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD)
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_SRC_ALPHA, GL_ZERO)
end

type AlphaBlendConstant <: AlphaBlendState
	color::(Float32, Float32, Float32, Float32)
end

function setstate(state::AlphaBlendConstant)
	glEnable(GL_BLEND)
	glBlendColor(state.color...)
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD)
	@assert glGetError() == GL_NO_ERROR
	glBlendFuncSeparate(GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR, GL_CONSTANT_ALPHA, GL_ZERO)
	@assert glGetError() == GL_NO_ERROR
end


type StencilStateDisabled <: StencilState
end

function setstate(::StencilStateDisabled)
	glDisable(GL_STENCIL_TEST)
end


type DepthStateDisabled <: DepthState
end

function setstate(::DepthStateDisabled)
	glDisable(GL_DEPTH_TEST)
end

type DepthStateLess <: DepthState
end

function setstate(::DepthStateLess)
	glEnable(GL_DEPTH_TEST)
	glDepthFunc(GL_LESS)
end


type CullStateDisabled <: CullState
end

function setstate(::CullStateDisabled)
	glDisable(GL_CULL_FACE)
end

type CullStateCCW <: CullState
end

function setstate(::CullStateCCW)
	glEnable(GL_CULL_FACE)
	# Poly orientation is CCW and we cull the back faces
	glFrontFace(GL_CCW)
	glCullFace(GL_BACK)
end


type RenderStateHolder
	# we store the states with their type's supertype as key, i.e. all the AlphaBlendStates will go to the same position
	states::Dict{DataType, RenderState}

	RenderStateHolder() = new(Dict{DataType, RenderState}())
end

function resetstate(holder::RenderStateHolder, state::RenderState) 
	@assert super(T) == RenderState
    delete!(holder.states, state)
end

function setstate(holder::RenderStateHolder, state::RenderState)
	holder.states[super(typeof(state))] = state
end

function getstate{T <: RenderState}(holder::RenderStateHolder, ::Type{T}, default)
	@assert super(T) == RenderState
	get(holder.states, T, default)
end

function set_and_apply(holder::RenderStateHolder, state::RenderState)
	key = super(typeof(state))
	if !haskey(holder.states, key) || holder.states[key] != state
		holder.states[key] = state
		setstate(state)
	end
end
