module WGPUTry

using GLFW
using WGPUNative
using CEnum

zero_ref!(ref::Ref{T}) where T = Base.unsafe_securezero!(Base.unsafe_convert(Ptr{T}, ref))
ptr_from_ref(ref::Ref{T}) where T = Base.unsafe_convert(Ptr{T}, ref)

function GetWin32Window(window)
    ccall((:glfwGetWin32Window, GLFW.libglfw), Ptr{Nothing}, (Ptr{GLFW.Window},), window.handle)
end

function GetModuleHandle(ptr)
    ccall((:GetModuleHandleA, "kernel32"), stdcall, Ptr{UInt32}, (UInt32,), ptr)
end

function CreateWin32Surface(wgpuInst::WGPUInstance, window::GLFW.Window, label::String)::WGPUSurface
    win32Desc = Ref(WGPUSurfaceDescriptorFromWindowsHWND(
        WGPUChainedStruct(C_NULL, WGPUSType_SurfaceDescriptorFromWindowsHWND),
        GetModuleHandle(C_NULL),
        GetWin32Window(window)
    ))

    surfDesc = Ref(WGPUSurfaceDescriptor(
        Ptr{WGPUChainedStruct}(Base.unsafe_convert(Ptr{Cvoid}, win32Desc)),
        pointer(label)
    ))

    GC.@preserve win32Desc label wgpuInstanceCreateSurface(wgpuInst, surfDesc)
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
    struct VSOut {
        @builtin(position) pos: vec4f,
        @location(0) color: vec4f,
    };

    @vertex
    fn vs_main(@location(0) pos: vec3f, @location(1) color: vec3f) -> VSOut {
        var vsOut: VSOut;
        vsOut.pos = vec4f(pos, 1.0);
        vsOut.color = vec4f(color, 1.0);
        return vsOut;
    }

    @fragment
    fn fs_main(vsOut: VSOut) -> @location(0) vec4f {
        return vsOut.color;
    }
    """

struct VertexPos
    pos::NTuple{3, Float32}
    color::NTuple{3, Float32}
end

const triVertices = [
    VertexPos((-0.5, -0.5, 0), (1, 0, 0)),
    VertexPos(( 0.5, -0.5, 0), (0, 1, 0)),
    VertexPos(( 0.0,  0.5, 0), (0, 0, 1)),
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

const entryVs = "vs_main"
const entryFs = "fs_main"

function CreateWGSLPipeline(device::WGPUDevice, name::String, source::String, vertexType::Type, surfFormat::WGPUTextureFormat)
    shader = CreateWGSLShaderModule(device, name, source)

    layoutDesc = Ref(WGPUPipelineLayoutDescriptor(
        C_NULL,
        pointer(name),
        0,
        C_NULL
    ))
    pipelineLayout = GC.@preserve name wgpuDeviceCreatePipelineLayout(device, layoutDesc)

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

function CreateBuffer(device::WGPUDevice, queue::WGPUQueue, name::String, usage::WGPUBufferUsageFlags, content)
    bufferDesc = Ref(WGPUBufferDescriptor(
        C_NULL,
        pointer(name),
        usage | WGPUBufferUsage_CopyDst,
        sizeof(content),
        false
    ))
    buffer = GC.@preserve name wgpuDeviceCreateBuffer(device, bufferDesc)
    GC.@preserve content wgpuQueueWriteBuffer(queue, buffer, 0, pointer(content, 1), bufferDesc[].size)
    buffer
end

function main()
    GLFW.Init()

    GLFW.WindowHint(GLFW.CLIENT_API, GLFW.NO_API)
    window = GLFW.CreateWindow(800, 600, "Win32 Kek")

    inst = wgpuCreateInstance(Ref(WGPUInstanceDescriptor(C_NULL)))
    surface = CreateWin32Surface(inst, window, "Keke")
    adapter = GetWGPUAdapter(inst, surface)

    adapterProps = GetWGPUAdapterProperties(adapter)
    @info unsafe_string(adapterProps[].name)

    device = GetWGPUDevice(adapter, "wgpu.jl")
    queue = wgpuDeviceGetQueue(device)

    surfFormat = wgpuSurfaceGetPreferredFormat(surface, adapter)
    @info "Surface format $surfFormat"

    pipeline = CreateWGSLPipeline(device, shaderName, shaderSrc, eltype(triVertices), surfFormat)

    vertexBuffer = CreateBuffer(device, queue, "triVerts", WGPUBufferUsageFlags(WGPUBufferUsage_Vertex), triVertices)

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
            @assert(texView != C_NULL)

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

    wgpuBufferRelease(vertexBuffer)
    wgpuRenderPipelineRelease(pipeline)
    wgpuQueueRelease(queue)
    wgpuDeviceRelease(device)
    wgpuAdapterRelease(adapter)
    wgpuSurfaceRelease(surface)
    wgpuInstanceRelease(inst)

    GLFW.DestroyWindow(window)
    GLFW.Terminate()
end

main()

end