module WGPUTry

using GLFW
using WGPUNative
using CEnum
using LinearAlgebra
import Base.zero

zero(::Type{NTuple{N,T}}) where {N,T}=ntuple(i->zero(T), N)
zero_ref!(ref::Ref{T}) where T = Base.unsafe_securezero!(Base.unsafe_convert(Ptr{T}, ref))
ptr_from_ref(ref::Ref{T}) where T = Base.unsafe_convert(Ptr{T}, ref)

function ptr_to_field(p::Ptr{T}, name::Symbol) where T
    fieldIndex = Base.fieldindex(T, name)
    fieldType = fieldtype(T, fieldIndex)
    fieldOffs = fieldoffset(T, fieldIndex)
    convert(Ptr{fieldType}, p + fieldOffs)
end
ptr_to_field(ref::Ref{T}, name::Symbol) where T = ptr_to_field(ptr_from_ref(ref), name)

get_ptr_field(p::Ptr{T}, name::Symbol) where T = unsafe_load(ptr_to_field(p, name))
get_ptr_field(p::Ref{T}, name::Symbol) where T = get_ptr_field(ptr_from_ref(p), name)
set_ptr_field!(p::Ptr{T}, name::Symbol, val) where T = unsafe_store!(ptr_to_field(p, name), val)
set_ptr_field!(p::Ref{T}, name::Symbol, val) where T = set_ptr_field!(ptr_from_ref(p), name, val)

function rotation_z(angle::Float32)::Matrix{Float32}
    c = cos(angle)
    s = sin(angle)
    Float32[ c s 0 0
            -s c 0 0
             0 0 1 0
             0 0 0 1 ]
end

if Sys.iswindows()
    function GetWin32Window(window)
        ccall((:glfwGetWin32Window, GLFW.libglfw), Ptr{Nothing}, (Ptr{GLFW.Window},), window.handle)
    end

    function GetModuleHandle(ptr)
        ccall((:GetModuleHandleA, "kernel32"), stdcall, Ptr{UInt32}, (UInt32,), ptr)
    end

    function GetOSSurfaceDescriptor(window::GLFW.Window)
        Ref(WGPUSurfaceDescriptorFromWindowsHWND(
            WGPUChainedStruct(C_NULL, WGPUSType_SurfaceDescriptorFromWindowsHWND),
            GetModuleHandle(C_NULL),
            GetWin32Window(window)
        ))
    end
elseif Sys.islinux()
    function GetX11Display()
        ccall((:glfwGetX11Display, GLFW.libglfw), Ptr{Nothing}, ())
    end

    function GetX11Window(window::GLFW.Window)
        ccall((:glfwGetX11Window, GLFW.libglfw), UInt64, (Ptr{GLFW.Window},), window.handle)
    end

    function GetOSSurfaceDescriptor(window::GLFW.Window)
        Ref(WGPUSurfaceDescriptorFromXlibWindow(
            WGPUChainedStruct(C_NULL, WGPUSType_SurfaceDescriptorFromXlibWindow),
            GetX11Display(),
            GetX11Window(window)
        ))
    end
else
    error("Unsupported OS")
end

function CreateOSSurface(wgpuInst::WGPUInstance, window::GLFW.Window, label::String)::WGPUSurface
    osDesc = GetOSSurfaceDescriptor(window)
    surfDesc = Ref(WGPUSurfaceDescriptor(
        Ptr{WGPUChainedStruct}(Base.unsafe_convert(Ptr{Cvoid}, osDesc)),
        pointer(label)
    ))

    GC.@preserve osDesc label wgpuInstanceCreateSurface(wgpuInst, surfDesc)
end

function GetWGPUAdapterCallback(status::WGPURequestAdapterStatus, adapter::WGPUAdapter, msg::Ptr{Cchar}, userData::Ptr{Cvoid})
    @assert(status == WGPURequestAdapterStatus_Success)
    adapterOut = Base.unsafe_pointer_to_objref(Ptr{WGPUAdapter}(userData))
    adapterOut[] = adapter
    nothing
end

function GetWGPUAdapter(wgpuInst::WGPUInstance, surface::WGPUSurface)::WGPUAdapter
    adapterOptions = Ref(WGPURequestAdapterOptions(
        C_NULL,
        surface,
        WGPUPowerPreference_HighPerformance,
        WGPUBackendType_Undefined,
        false # forceFallbackAdapter
    ))

    adapter = Ref{WGPUAdapter}()
    callback = @cfunction(GetWGPUAdapterCallback, Cvoid, (WGPURequestAdapterStatus, WGPUAdapter, Ptr{Cchar}, Ptr{Cvoid}))
    wgpuInstanceRequestAdapter(wgpuInst, adapterOptions, callback, adapter)
    adapter[]
end

function GetWGPUAdapterProperties(adapter::WGPUAdapter)::Ref{WGPUAdapterProperties}
    adapterProps = Ref{WGPUAdapterProperties}()
    zero_ref!(adapterProps)
    ret = wgpuAdapterGetProperties(adapter, adapterProps)
    @assert(ret != 0)
    adapterProps
end

function GetWGPUAdapterLimits(adapter::WGPUAdapter)::Ref{WGPULimits}
    adapterLimits = Ref{WGPUSupportedLimits}()
    zero_ref!(adapterLimits)
    ret = wgpuAdapterGetLimits(adapter, adapterLimits)
    @assert(ret != 0)
    Ref(adapterLimits[].limits)
end

function GetWGPUAdapterFeatures(adapter::WGPUAdapter)::Vector{WGPUFeatureName}
    adapterFeatures = Vector{WGPUFeatureName}()
    resize!(adapterFeatures, wgpuAdapterEnumerateFeatures(adapter, C_NULL))
    size = GC.@preserve adapterFeatures wgpuAdapterEnumerateFeatures(adapter, pointer(adapterFeatures, 1))
    @assert(size == length(adapterFeatures))
    adapterFeatures
end

function GetWGPUDeviceCallback(status::WGPURequestDeviceStatus, device::WGPUDevice, msg::Ptr{Cchar} , userData::Ptr{Cvoid})
    @assert(status == WGPURequestDeviceStatus_Success)
    deviceOut = Base.unsafe_pointer_to_objref(Ptr{WGPUDevice}(userData))
    deviceOut[] = device
    nothing
end

function GetWGPUDevice(adapter::WGPUAdapter, label::String)::WGPUDevice
    deviceDesc = Ref(WGPUDeviceDescriptor(
        C_NULL,
        pointer(label),
        0,
        C_NULL,
        C_NULL,
        WGPUQueueDescriptor(
            C_NULL,
            pointer(label)
        ),
        C_NULL,
        C_NULL
    ))

    device = Ref{WGPUDevice}()
    callback = @cfunction(GetWGPUDeviceCallback, Cvoid, (WGPURequestDeviceStatus, WGPUDevice, Ptr{Cchar}, Ptr{Cvoid}))
    GC.@preserve label wgpuAdapterRequestDevice(adapter, deviceDesc, callback, device)
    device[]
end

function ConfigureWGPUSurface(surface::WGPUSurface, device::WGPUDevice, size::NTuple{2, UInt32}, surfFormat::WGPUTextureFormat)
    viewFormats = [surfFormat]

    config = Ref(WGPUSurfaceConfiguration(
        C_NULL,
        device,
        surfFormat,
        WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_CopySrc,
        length(viewFormats),
        pointer(viewFormats, 1),
        WGPUCompositeAlphaMode_Opaque,
        size[1],
        size[2],
        WGPUPresentMode_Fifo
    ))

    GC.@preserve viewFormats wgpuSurfaceConfigure(surface, config)
end

function CreateSurfaceCurrentTextureView(surface::WGPUSurface)
    surfTex = Ref{WGPUSurfaceTexture}()
    zero_ref!(surfTex)
    wgpuSurfaceGetCurrentTexture(surface, surfTex)

    if surfTex[].status != WGPUSurfaceGetCurrentTextureStatus_Success 
        return C_NULL
    end

    @assert(surfTex[].texture != C_NULL)
    @assert(surfTex[].status == WGPUSurfaceGetCurrentTextureStatus_Success)

    wgpuTextureCreateView(surfTex[].texture, C_NULL)
end

const shaderName = "#tri.wgsl"
const shaderSrc = 
    """
    struct Uniforms {
        worldViewProj: mat4x4f,
    };

    @group(0) @binding(0) var<uniform> uni: Uniforms;
    @group(0) @binding(1) var texSampler: sampler;
    @group(0) @binding(2) var tex0: texture_2d<f32>;
   
    struct VSOut {
        @builtin(position) pos: vec4f,
        @location(0) color: vec4f,
        @location(1) uv: vec2f,
    };

    @vertex
    fn vs_main(@location(0) pos: vec3f, @location(1) color: vec3f, @location(2) uv: vec2f) -> VSOut {
        var vsOut: VSOut;
        vsOut.pos = uni.worldViewProj * vec4f(pos, 1.0);
        vsOut.color = vec4f(color, 1.0);
        vsOut.uv = uv;
        return vsOut;
    }

    @fragment
    fn fs_main(vsOut: VSOut) -> @location(0) vec4f {
        var tex: vec4f = textureSample(tex0, texSampler, vsOut.uv);
        return vsOut.color * tex;
    }
    """

struct Uniforms
    worldViewProj::NTuple{16, Float32}
end

struct VertexPos
    pos::NTuple{3, Float32}
    color::NTuple{3, Float32}
    uv::NTuple{2, Float32}
end

const triVertices = [
    VertexPos((-0.5, -0.5, 0), (1, 0, 0), (0, 0)),
    VertexPos(( 0.5, -0.5, 0), (0, 1, 0), (0, 1)),
    VertexPos(( 0.0,  0.5, 0), (0, 0, 1), (1, 1)),
]

function CreateWGSLShaderModule(device::WGPUDevice, label::String, source::String)::WGPUShaderModule
    sourceDesc = Ref(WGPUShaderModuleWGSLDescriptor(
        WGPUChainedStruct(C_NULL, WGPUSType_ShaderModuleWGSLDescriptor),
        pointer(source)
    ))
    shaderDesc = Ref(WGPUShaderModuleDescriptor(
        Ptr{WGPUChainedStruct}(Base.unsafe_convert(Ptr{Cvoid}, sourceDesc)),
        pointer(label),
        0,
        C_NULL
    ))
    GC.@preserve label source sourceDesc wgpuDeviceCreateShaderModule(device, shaderDesc)
end

function GetVertexFormat(::Type{T}, unorm::Bool = false)::WGPUVertexFormat where T
    local baseType
    dim = fieldcount(T)
    elemType = eltype(T)
    if elemType == Float32
        baseType = "Float32"
    elseif elemType == UInt8
        baseType = unorm ? "Unorm8" : "Uint8"
    elseif elemType == Int8
        baseType = unorm ? "Snorm8" : "Sint8"
    elseif elemType == UInt16
        baseType = unorm ? "Unorm16" : "Uint16"
    elseif elemType == Int16
        baseType = unorm ? "Snorm16" : "Sint16"
    elseif elemType == UInt32
        baseType = "Uint32"
    elseif elemType == Int32
        baseType = "Sint32"
    else
        error("Unrecognized type")
    end
    valName = "WGPUVertexFormat_$baseType"
    if dim > 1
        valName = "$(valName)x$dim"
    end
    valName = Symbol(valName)
    ind = findfirst(x->x[1] == valName, CEnum.name_value_pairs(WGPUVertexFormat))
    isnothing(ind) ? WGPUVertexFormat_Undefined : WGPUVertexFormat(CEnum.name_value_pairs(WGPUVertexFormat)[ind][2]) 
end

function FillVertexAttributes(::Type{T}, attrs::Vector{WGPUVertexAttribute}) where T
    for i in 1:fieldcount(T)
        vertexFormat = GetVertexFormat(fieldtype(T, i))
        @assert(vertexFormat != WGPUVertexFormat_Undefined)
        push!(attrs, WGPUVertexAttribute(
            vertexFormat,
            fieldoffset(T, i),
            length(attrs)
        ))
    end
end

function GetVertexLayout(::Type{T}, attrs::Vector{WGPUVertexAttribute})::WGPUVertexBufferLayout where T
    FillVertexAttributes(T, attrs)
    WGPUVertexBufferLayout(
        sizeof(T),
        WGPUVertexStepMode_Vertex,
        length(attrs),
        pointer(attrs, 1)
    )
end

function CreateBindGroupLayout(device::WGPUDevice, name::String, stageVisibility::WGPUShaderStageFlags, bindingLayouts::Vector{Any})::WGPUBindGroupLayout
    groupLayoutEntries = WGPUBindGroupLayoutEntry[]
    resize!(groupLayoutEntries, length(bindingLayouts))
    Base.memset(pointer(groupLayoutEntries, 1), 0, sizeof(groupLayoutEntries))
    for i in 1:length(bindingLayouts)
        entry = pointer(groupLayoutEntries, i)
        set_ptr_field!(entry, :binding, i - 1)
        set_ptr_field!(entry, :visibility, stageVisibility)
        bindType = typeof(bindingLayouts[i])
        typeFound = false
        for field in (:buffer, :sampler, :texture, :storageTexture)
            if bindType == fieldtype(WGPUBindGroupLayoutEntry, field)
                set_ptr_field!(entry, field, bindingLayouts[i])
                typeFound = true
                break
            end
        end
        @assert(typeFound)
    end
    bindGroupDesc = Ref(WGPUBindGroupLayoutDescriptor(
        C_NULL,
        pointer(name),
        length(groupLayoutEntries),
        pointer(groupLayoutEntries, 1)
    ))
    GC.@preserve name groupLayoutEntries wgpuDeviceCreateBindGroupLayout(device, bindGroupDesc)
end

function CreateBindGroup(device::WGPUDevice, name::String, bindGroupLayout::WGPUBindGroupLayout, bindings::Vector{Any})::WGPUBindGroup
    bindGroupEntries = WGPUBindGroupEntry[]
    resize!(bindGroupEntries, length(bindings))
    Base.memset(pointer(bindGroupEntries, 1), 0, sizeof(bindGroupEntries))
    for i in 1:length(bindings)
        entry = pointer(bindGroupEntries, i)
        set_ptr_field!(entry, :binding, i - 1)
        bind = bindings[i]
        if typeof(bind) == WGPUBuffer
            set_ptr_field!(entry, :buffer, bind)
            set_ptr_field!(entry, :size, wgpuBufferGetSize(bind))
        elseif typeof(bind) == WGPUSampler
            set_ptr_field!(entry, :sampler, bind)
        elseif typeof(bind) == WGPUTextureView
            set_ptr_field!(entry, :textureView, bind)
        else
            error("Unrecognized binding")
        end
    end
    bindGroupDesc = Ref(WGPUBindGroupDescriptor(
        C_NULL,
        pointer(name),
        bindGroupLayout,
        length(bindGroupEntries),
        pointer(bindGroupEntries, 1)
    ))
    GC.@preserve name bindGroupEntries wgpuDeviceCreateBindGroup(device, bindGroupDesc)
end    

const entryVs = "vs_main"
const entryFs = "fs_main"

function CreateWGSLRenderPipeline(device::WGPUDevice, name::String, source::String, vertexType::Type, bindGroupLayouts::Vector{WGPUBindGroupLayout}, surfFormat::WGPUTextureFormat)::WGPURenderPipeline
    shader = CreateWGSLShaderModule(device, name, source)

    layoutDesc = Ref(WGPUPipelineLayoutDescriptor(
        C_NULL,
        pointer(name),
        length(bindGroupLayouts),
        pointer(bindGroupLayouts, 1)
    ))
    pipelineLayout = GC.@preserve name bindGroupLayouts wgpuDeviceCreatePipelineLayout(device, layoutDesc)

    colorTargets = [WGPUColorTargetState(
        C_NULL,
        surfFormat,
        C_NULL,
        WGPUColorWriteMask_All
    )]
    fragmentState = Ref(WGPUFragmentState(
        C_NULL,
        shader,
        pointer(entryFs),
        0,
        C_NULL,
        length(colorTargets),
        pointer(colorTargets, 1)
    ))
    vertexAttrs = WGPUVertexAttribute[]
    vertexLayout = [GetVertexLayout(vertexType, vertexAttrs)]
    pipelineDesc = Ref(WGPURenderPipelineDescriptor(
        C_NULL,
        pointer(name),
        pipelineLayout,
        WGPUVertexState(
            C_NULL,
            shader,
            pointer(entryVs),
            0,
            C_NULL,
            length(vertexLayout),
            pointer(vertexLayout, 1)
        ),
        WGPUPrimitiveState(
            C_NULL,
            WGPUPrimitiveTopology_TriangleList,
            WGPUIndexFormat_Undefined,
            WGPUFrontFace_CCW,
            WGPUCullMode_None
        ),
        C_NULL,
        WGPUMultisampleState(
            C_NULL,
            1,
            typemax(UInt32),
            false
        ),
        ptr_from_ref(fragmentState)
    ))
    pipeline = GC.@preserve name entryVs entryFs colorTargets vertexAttrs vertexLayout fragmentState wgpuDeviceCreateRenderPipeline(device, pipelineDesc)

    wgpuPipelineLayoutRelease(pipelineLayout)
    wgpuShaderModuleRelease(shader)

    pipeline
end

function CreateBuffer(device::WGPUDevice, queue::WGPUQueue, name::String, usage::WGPUBufferUsageFlags, content)::WGPUBuffer
    bufferDesc = Ref(WGPUBufferDescriptor(
        C_NULL,
        pointer(name),
        usage | WGPUBufferUsage_CopyDst,
        sizeof(content),
        false
    ))
    buffer = GC.@preserve name wgpuDeviceCreateBuffer(device, bufferDesc)
    ptrContent = typeof(content) <: Ref ? ptr_from_ref(content) : pointer(content)
    GC.@preserve content wgpuQueueWriteBuffer(queue, buffer, 0, ptrContent, bufferDesc[].size)
    buffer
end

function CreateSampler(device::WGPUDevice, name::String, addressMode::WGPUAddressMode, filterMode::WGPUFilterMode)::WGPUSampler
    samplerDesc = Ref(WGPUSamplerDescriptor(
        C_NULL,
        pointer(name),
        addressMode,
        addressMode,
        addressMode,
        filterMode,
        filterMode,
        filterMode == WGPUFilterMode_Nearest ? WGPUMipmapFilterMode_Nearest : WGPUMipmapFilterMode_Linear,
        0,
        typemax(Float32),
        WGPUCompareFunction_Undefined,
        filterMode == WGPUFilterMode_Nearest ? 1 : typemax(UInt16)
    ))
    GC.@preserve name wgpuDeviceCreateSampler(device, samplerDesc)
end

function TypeToTextureFormat(::Type{T}) where T
    if T == UInt8
        return WGPUTextureFormat_R8Unorm
    end
    if T == NTuple{4, UInt8}
        return WGPUTextureFormat_RGBA8Unorm
    end
    error("Unknown type for texture format")
end

function CreateTexture(device::WGPUDevice, queue::WGPUQueue, name::String, usage::WGPUTextureUsage, content)::WGPUTexture
    contentDim = [size(content)..., 1, 1]
    textureDesc = Ref(WGPUTextureDescriptor(
        C_NULL,
        pointer(name),
        WGPUTextureUsage(usage | WGPUTextureUsage_CopyDst),
        WGPUTextureDimension(WGPUTextureDimension_1D + ndims(content) - 1),
        WGPUExtent3D(contentDim[1], contentDim[2], contentDim[3]),
        TypeToTextureFormat(eltype(content)),
        UInt32(ceil(log2(maximum(size(content))))),
        1,
        0,
        C_NULL
    ))
    texture = GC.@preserve name wgpuDeviceCreateTexture(device, textureDesc)
    imageCopyTex = Ref(WGPUImageCopyTexture(
        C_NULL,
        texture,
        0,
        WGPUOrigin3D(0, 0, 0),
        WGPUTextureAspect_All
    ))
    texLayout = Ref(WGPUTextureDataLayout(
        C_NULL,
        0,
        contentDim[1] * sizeof(eltype(content)),
        contentDim[1] * contentDim[2] * sizeof(eltype(content))
    ))
    GC.@preserve textureDesc, wgpuQueueWriteTexture(queue, imageCopyTex, pointer(content), sizeof(content), texLayout, ptr_to_field(textureDesc, :size))
    texture
end

function LogCallback(logLevel::WGPULogLevel, msg::Ptr{Cchar})
    @info logLevel unsafe_string(msg)
end

function main()
    GLFW.WindowHint(GLFW.CLIENT_API, GLFW.NO_API)
    window = GLFW.CreateWindow(800, 800, "Win32 Kek")

    # the @cfunction value needs to stay local because otherwise the Julia debugger breaks
    CLogCallback = @cfunction(LogCallback, Cvoid, (WGPULogLevel, Ptr{Cchar}))
    wgpuSetLogCallback(CLogCallback, C_NULL)
    wgpuSetLogLevel(WGPULogLevel_Warn)

    inst = wgpuCreateInstance(Ref(WGPUInstanceDescriptor(C_NULL)))

    surface = CreateOSSurface(inst, window, "Keke")
    adapter = GetWGPUAdapter(inst, surface)

    adapterProps = GetWGPUAdapterProperties(adapter)
    @info unsafe_string(adapterProps[].name)

    device = GetWGPUDevice(adapter, "wgpu.jl")
    queue = wgpuDeviceGetQueue(device)

    surfFormat = wgpuSurfaceGetPreferredFormat(surface, adapter)
    @info "Surface format $surfFormat"

    bindGroupLayout = CreateBindGroupLayout(device, shaderName, WGPUShaderStageFlags(WGPUShaderStage_Vertex | WGPUShaderStage_Fragment), Any[
        WGPUBufferBindingLayout(C_NULL, WGPUBufferBindingType_Uniform, false, 0),
        WGPUSamplerBindingLayout(C_NULL, WGPUSamplerBindingType_Filtering),
        WGPUTextureBindingLayout(C_NULL, WGPUTextureSampleType_Float, WGPUTextureViewDimension_2D, false),
    ])

    pipeline = CreateWGSLRenderPipeline(device, shaderName, shaderSrc, eltype(triVertices), [bindGroupLayout], surfFormat)

    uniforms = Ref{Uniforms}()
    set_ptr_field!(uniforms, :worldViewProj, tuple(reshape(Matrix{Float32}(I, 4, 4), 16)...))
    
    uniformBuffer = CreateBuffer(device, queue, "uniforms", WGPUBufferUsageFlags(WGPUBufferUsage_Uniform), uniforms)
    vertexBuffer = CreateBuffer(device, queue, "triVerts", WGPUBufferUsageFlags(WGPUBufferUsage_Vertex), triVertices)

    samplerLinearRepeat = CreateSampler(device, "linearRepeat", WGPUAddressMode_Repeat, WGPUFilterMode_Linear)
    texture = CreateTexture(device, queue, "tex2D", WGPUTextureUsage_TextureBinding, [ntuple(i->UInt8((isodd(x+y) || i > 3) * 255), 4) for x=1:4, y=1:4])
    textureView = wgpuTextureCreateView(texture, C_NULL)

    bindGroup = CreateBindGroup(device, "bindGroup", bindGroupLayout, Any[uniformBuffer, samplerLinearRepeat, textureView])

    startTime = time()
    frames = 0
    (winWidth, winHeight) = (-1, -1)    
    while !GLFW.WindowShouldClose(window)
        (newWidth, newHeight) = GLFW.GetWindowSize(window)
        if newWidth != winWidth || newHeight != winHeight
            (winWidth, winHeight) = (newWidth, newHeight)
            ConfigureWGPUSurface(surface, device, (UInt32(winWidth), UInt32(winHeight)), surfFormat)
        end

        texView = CreateSurfaceCurrentTextureView(surface)
        if texView != C_NULL
            # updates
            rot = rotation_z(Float32((time() - startTime) % 2pi))
            Base.memmove(ptr_to_field(uniforms, :worldViewProj), pointer(rot), sizeof(rot))
            wgpuQueueWriteBuffer(queue, uniformBuffer, 0, uniforms, sizeof(uniforms))

            label = "cmds"
            encoderDesc = Ref(WGPUCommandEncoderDescriptor(C_NULL, pointer(label)))
            encoder = GC.@preserve label wgpuDeviceCreateCommandEncoder(device, encoderDesc)

            colorAttachments = [WGPURenderPassColorAttachment(
                C_NULL,
                texView,
                C_NULL,
                WGPULoadOp_Clear,
                WGPUStoreOp_Store,
                WGPUColor(0.3, 0.3, 0.3, 1)
            )] 
            renderPassDesc = Ref(WGPURenderPassDescriptor(
                C_NULL,
                pointer(label),
                length(colorAttachments),
                pointer(colorAttachments, 1),
                C_NULL,
                C_NULL,
                C_NULL
            ))
            renderPass = GC.@preserve colorAttachments wgpuCommandEncoderBeginRenderPass(encoder, renderPassDesc)

            #rendering goes here
            wgpuRenderPassEncoderSetPipeline(renderPass, pipeline)
            wgpuRenderPassEncoderSetBindGroup(renderPass, 0, bindGroup, 0, C_NULL)
            wgpuRenderPassEncoderSetVertexBuffer(renderPass, 0, vertexBuffer, 0, wgpuBufferGetSize(vertexBuffer))
            wgpuRenderPassEncoderDraw(renderPass, 3, 1, 0, 0)

            wgpuRenderPassEncoderEnd(renderPass)
            wgpuRenderPassEncoderRelease(renderPass)

            cmdBufferDesc = Ref(WGPUCommandBufferDescriptor(C_NULL, pointer(label)))
            cmdBuffer = GC.@preserve label wgpuCommandEncoderFinish(encoder, cmdBufferDesc)
            wgpuCommandEncoderRelease(encoder)

            cmds = [cmdBuffer]
            wgpuQueueSubmit(queue, length(cmds), pointer(cmds, 1))
            wgpuCommandBufferRelease(cmdBuffer)

            wgpuTextureViewRelease(texView)
            wgpuSurfacePresent(surface)
            
            frames += 1
        end
        wgpuDevicePoll(device, false, C_NULL)
        GLFW.PollEvents()
    end
    runTime = time() - startTime
    @info "Frames: $frames, run time: $(round(runTime; digits = 3)), fps: $(round(frames / runTime; digits = 3))"

    wgpuBindGroupRelease(bindGroup)
    wgpuTextureViewRelease(textureView)
    wgpuTextureRelease(texture)
    wgpuSamplerRelease(samplerLinearRepeat)
    wgpuBufferRelease(uniformBuffer)
    wgpuBufferRelease(vertexBuffer)
    wgpuRenderPipelineRelease(pipeline)
    wgpuBindGroupLayoutRelease(bindGroupLayout)
    wgpuQueueRelease(queue)
    wgpuDeviceRelease(device)
    wgpuAdapterRelease(adapter)
    wgpuSurfaceRelease(surface)
    wgpuInstanceRelease(inst)

    wgpuSetLogCallback(C_NULL, C_NULL)

    GLFW.DestroyWindow(window)
end

try
    GLFW.Init()
    main()
finally
    # so that windows close in case of an runtime error
    GLFW.Terminate()
end

end