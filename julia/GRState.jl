abstract RenderState
abstract AlphaBlendState <: RenderState
abstract StencilState <: RenderState
abstract DepthState <: RenderState
abstract CullState <: RenderState


type AlphaBlendDisabled <: AlphaBlendState
end

function setstate(::AlphaBlendDisabled)
	glDisable(BLEND)
end

type AlphaBlendSrcAlpha <: AlphaBlendState
end

function setstate(::AlphaBlendSrcAlpha)
	glEnable(BLEND)
	glBlendEquationSeparate(FUNC_ADD, FUNC_ADD)
	glBlendFuncSeparate(SRC_ALPHA, ONE_MINUS_SRC_ALPHA, SRC_ALPHA, ZERO)
end

type AlphaBlendAdditive <: AlphaBlendState
end

function setstate(::AlphaBlendAdditive)
	glEnable(BLEND)
	glBlendEquationSeparate(FUNC_ADD, FUNC_ADD)
	glBlendFuncSeparate(SRC_ALPHA, ONE, SRC_ALPHA, ZERO)
end

type AlphaBlendConstant <: AlphaBlendState
	color::(Float32, Float32, Float32, Float32)
end

function setstate(state::AlphaBlendConstant)
	glEnable(BLEND)
	glBlendColor(state.color...)
	glBlendEquationSeparate(FUNC_ADD, FUNC_ADD)
	@assert glGetError() == NO_ERROR
	glBlendFuncSeparate(CONSTANT_COLOR, ONE_MINUS_CONSTANT_COLOR, CONSTANT_ALPHA, ZERO)
	@assert glGetError() == NO_ERROR
end


type StencilStateDisabled <: StencilState
end

function setstate(::StencilStateDisabled)
	glDisable(STENCIL_TEST)
end


type DepthStateDisabled <: DepthState
end

function setstate(::DepthStateDisabled)
	glDisable(DEPTH_TEST)
end

type DepthStateLess <: DepthState
end

function setstate(::DepthStateLess)
	glEnable(DEPTH_TEST)
	glDepthFunc(LESS)
end


type CullStateDisabled <: CullState
end

function setstate(::CullStateDisabled)
	glDisable(CULL_FACE)
end

type CullStateCCW <: CullState
end

function setstate(::CullStateCCW)
	glEnable(CULL_FACE)
	# Poly orientation is CCW and we cull the back faces
	glFrontFace(CCW)
	glCullFace(BACK)
end


type RenderStateHolder
	# we store the states with their type's supertype as key, i.e. all the AlphaBlendStates will go to the same position
	states::Dict{DataType, RenderState}

	RenderStateHolder() = new(Dict{DataType, RenderState}())
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
