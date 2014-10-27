type Renderer
	camera::Camera
    resources::Set{Renderable}
    renderState::RenderStateHolder
    toRender::Vector{Renderable}
    sortFunc::Function
    clearColor::Union((Float32, Float32, Float32, Float32), Nothing)
    clearStencil::Union(Int, Nothing)
    clearDepth::Union(Float64, Nothing)

    Renderer() = new(Camera(), Set{Renderable}(), RenderStateHolder(), Array(Renderable, 0), identity, (0f0, 0f0, 0f0, 1f0), 0, 0)
end

global renderer_instance = nothing

renderer() = renderer_instance::Union{Renderer, Nothing}

function init(renderer::Renderer)
    @assert renderer_instance == nothing
    renderer_instance = renderer
end

function done(renderer::Renderer)
    for r in renderer.resources
        done(r)
    end
    empty!(renderer.resources)
    renderer_instance = nothing
end

function add(renderer::Renderer, resource::Resource)
    push!(renderer.resources, resource)
end

function remove(renderer::Renderer, resource::Resource)
    pop!(renderer.resources, resource)
end

function add(renderer::Renderer, renderable::Renderable)
    push!(renderer.toRender, renderable)
end

setClearColor(renderer::Renderer, c) = renderer.clearColor = c
setClearStencil(renderer::Renderer, s) = renderer.clearStencil = s
setClearDepth(renderer::Renderer, d) = renderer.clearDepth = d

function render_frame(renderer::Renderer)
    gl_clear_buffers(renderer.clearColor, renderer.clearStencil, renderer.clearDepth)
    # cull
    frustum = getfrustum(renderer.camera)
    filter!(r->!outside(frustum, getbound(r)), renderer.toRender)
    # sort
    if renderer.sortFunc != identity
        sort!(toRender, lt = renderer.sortFunc)
    end
    # render
    for r in renderer.toRender
        render(r, renderer)
    end

    empty!(toRender)
end

function apply(holder::RenderStateHolder, renderer::Renderer) 
    for state in values(holder.states)
        set_and_apply(renderer.renderState, state)
    end
end
