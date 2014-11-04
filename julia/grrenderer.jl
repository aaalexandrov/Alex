type Renderer
	camera::Camera
    resources::Dict{Symbol, Resource}
    renderState::RenderStateHolder
    toRender::Vector{Renderable}
    sortFunc::Function
    clearColor::Union((Float32, Float32, Float32, Float32), Nothing)
    clearStencil::Union(Int, Nothing)
    clearDepth::Union(Float64, Nothing)

    Renderer() = new(Camera(), Dict{Symbol, Resource}(), RenderStateHolder(), Array(Renderable, 0), identity, (0f0, 0f0, 0f0, 1f0), 0, 0.0)
end

global renderer_instance = nothing

renderer() = renderer_instance::Union(Renderer, Nothing)

function init(renderer::Renderer)
    @assert renderer_instance == nothing
    global renderer_instance = renderer
end

function done(renderer::Renderer)
    for r in values(renderer.resources)
        done(r)
    end
    empty!(renderer.resources)
    global renderer_instance = nothing
end

function add_resource(renderer::Renderer, resource::Resource)
    if haskey(renderer.resources, resource.id)
        resource.id = symbol("$(resource.id)_$(object_id(resource))")
        @assert !haskey(renderer.resources, resource.id)
    end
    renderer.resources[resource.id] = resource
end

function remove_resource(renderer::Renderer, resource::Resource)
    delete!(renderer.resources, resource.id)
end

get_resource(renderer::Renderer, id::Symbol) = get(renderer.resources, id, nothing)

function init_resource{T}(renderer::Renderer, ::Type{T}, params...; id::Symbol = :none)
    res = T()
    if id == :none
        init(res, params...)
    else
        init(res, params..., id = id)
    end
    add_resource(renderer, res)
end

function done_resource(renderer::Renderer, resource::Resource)
    remove_resource(renderer, resource)
    done(resource)
end

function add(renderer::Renderer, renderable::Renderable)
    push!(renderer.toRender, renderable)
end

set_clear_color(renderer::Renderer, c) = renderer.clearColor = c
set_clear_stencil(renderer::Renderer, s) = renderer.clearStencil = s
set_clear_depth(renderer::Renderer, d) = renderer.clearDepth = d

getcamera(renderer::Renderer) = renderer.camera

function render_frame(renderer::Renderer)
    gl_clear_buffers(renderer.clearColor, renderer.clearStencil, renderer.clearDepth)
    # cull
    frustum = getfrustum(renderer.camera)
    filter!(r->!Shapes.outside(frustum, getbound(r)), renderer.toRender)
    # sort
    if renderer.sortFunc != identity
        sort!(renderer.toRender, lt = renderer.sortFunc)
    end
    # render
    for r in renderer.toRender
        render(r, renderer)
    end

    empty!(renderer.toRender)
end

function apply(holder::RenderStateHolder, renderer::Renderer) 
    for state in values(holder.states)
        set_and_apply(renderer.renderState, state)
    end
end
