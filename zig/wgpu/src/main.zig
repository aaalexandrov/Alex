const std = @import("std");
const glfw = @import("zglfw");
const wgpu = @import("wgpu");
const zm = @import("zmath");

const PlainUniforms = extern struct {
    worldViewProj: zm.Mat = zm.identity(),
};

const PlainVertexPosColorUv = extern struct {
    pos: [3]f32,
    color: [3]f32,
    uv: [2]f32,
};

const PlainVertices = [_]PlainVertexPosColorUv{
    .{ .pos = .{ -0.5, -0.5, 0.0 }, .color = .{ 1, 0, 0 }, .uv = .{ 0, 0 } },
    .{ .pos = .{ 0.5, -0.5, 0.0 }, .color = .{ 0, 1, 0 }, .uv = .{ 0, 1 } },
    .{ .pos = .{ 0.0, 0.5, 0.0 }, .color = .{ 0, 0, 1 }, .uv = .{ 1, 1 } },
};

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    const alloc = gpa.allocator();
    defer {
        const deinit_status = gpa.deinit();
        if (deinit_status == .leak)
            @panic("Memory leaks detected!");
    }

    wgpu.setLogCallback(logCallback, null);
    wgpu.setLogLevel(.warn);

    const instance = wgpu.Instance.create(null).?;
    defer instance.release();

    try glfw.init();
    defer glfw.terminate();

    const windowName = "Wgpu Thin";
    glfw.windowHint(.client_api, .no_api);
    const window = try glfw.Window.create(600, 600, windowName, null);
    defer window.destroy();

    const chained = getSurfaceChain(window);
    const surface = instance.createSurface(&wgpu.SurfaceDescriptor{
        .next_in_chain = &chained.chain,
        .label = windowName,
    }).?;
    defer surface.release();

    const adapter = instance.requestAdapterSync(&wgpu.RequestAdapterOptions{
        .compatible_surface = surface,
        .power_preference = .high_performance,
    }).adapter.?;
    defer adapter.release();

    var adapterInfo: wgpu.AdapterInfo = undefined;
    adapter.getInfo(&adapterInfo);
    defer adapterInfo.freeMembers();
    std.debug.print("Adapter: {s}, type: {?s}, backend: {?s}\n", .{ adapterInfo.device, std.enums.tagName(wgpu.AdapterType, adapterInfo.adapter_type), std.enums.tagName(wgpu.BackendType, adapterInfo.backend_type) });

    const surfaceFormat = getSurfaceFormat(surface, adapter).?;
    std.debug.print("Surface format: {?s}\n", .{std.enums.tagName(wgpu.TextureFormat, surfaceFormat)});

    const device = adapter.requestDeviceSync(&wgpu.DeviceDescriptor{
        .required_limits = null,
    }).device.?;
    defer device.release();

    const queue = device.getQueue().?;
    defer queue.release();

    const plainShader = try createShaderModule(device, "data/plain.wgsl", alloc);
    defer plainShader.release();

    const plainBindGroupLayout = device.createBindGroupLayout(&wgpu.BindGroupLayoutDescriptor{
        .label = "Plain",
        .entry_count = 3,
        .entries = &[_]wgpu.BindGroupLayoutEntry{
            .{
                .binding = 0,
                .visibility = wgpu.ShaderStage.vertex | wgpu.ShaderStage.fragment,
                .buffer = .{
                    .type = .uniform,
                },
                .sampler = .{},
                .storage_texture = .{},
                .texture = .{},
            },
            .{
                .binding = 1,
                .visibility = wgpu.ShaderStage.vertex | wgpu.ShaderStage.fragment,
                .buffer = .{},
                .sampler = .{
                    .type = .filtering,
                },
                .storage_texture = .{},
                .texture = .{},
            },
            .{
                .binding = 2,
                .visibility = wgpu.ShaderStage.vertex | wgpu.ShaderStage.fragment,
                .buffer = .{},
                .sampler = .{},
                .storage_texture = .{},
                .texture = .{
                    .sample_type = .float,
                },
            },
        },
    }).?;
    defer plainBindGroupLayout.release();

    const plainPipelineLayout = device.createPipelineLayout(&wgpu.PipelineLayoutDescriptor{
        .label = "Plain",
        .bind_group_layout_count = 1,
        .bind_group_layouts = &[_]*wgpu.BindGroupLayout{
            plainBindGroupLayout,
        },
    }).?;
    defer plainPipelineLayout.release();

    const plainPipeline = device.createRenderPipeline(&wgpu.RenderPipelineDescriptor{
        .label = "plain",
        .layout = plainPipelineLayout,
        .vertex = .{
            .module = plainShader,
            .entry_point = "vs_main",
            .buffer_count = 1,
            .buffers = &[_]wgpu.VertexBufferLayout{
                .{
                    .array_stride = @sizeOf(PlainVertexPosColorUv),
                    .attribute_count = 3,
                    .attributes = &[_]wgpu.VertexAttribute{
                        .{ .format = .float32x3, .offset = 0, .shader_location = 0 },
                        .{ .format = .float32x3, .offset = 12, .shader_location = 1 },
                        .{ .format = .float32x2, .offset = 24, .shader_location = 2 },
                    },
                },
            },
        },
        .primitive = .{},
        .multisample = .{},
        .fragment = &wgpu.FragmentState{
            .module = plainShader,
            .entry_point = "fs_main",
            .target_count = 1,
            .targets = &[_]wgpu.ColorTargetState{
                .{
                    .format = surfaceFormat,
                },
            },
        },
    }).?;
    defer plainPipeline.release();

    const plainVerticesBuffer = device.createBuffer(&wgpu.BufferDescriptor{
        .label = "PlainVertices",
        .usage = wgpu.BufferUsage.vertex | wgpu.BufferUsage.copy_dst,
        .size = std.mem.sliceAsBytes(&PlainVertices).len,
    }).?;
    defer plainVerticesBuffer.release();
    queue.writeBuffer(plainVerticesBuffer, 0, (&PlainVertices).ptr, std.mem.sliceAsBytes(&PlainVertices).len);

    var plainUniforms = PlainUniforms{};
    const plainUniformsBuffer = device.createBuffer(&wgpu.BufferDescriptor{
        .label = "PlainUniforms",
        .usage = wgpu.BufferUsage.uniform | wgpu.BufferUsage.copy_dst,
        .size = @sizeOf(PlainUniforms),
    }).?;
    defer plainUniformsBuffer.release();
    queue.writeBuffer(plainUniformsBuffer, 0, &plainUniforms, @sizeOf(PlainUniforms));

    const linearRepeatSampler = device.createSampler(&wgpu.SamplerDescriptor{
        .label = "LinearRepeat",
        .address_mode_u = .repeat,
        .address_mode_v = .repeat,
        .address_mode_w = .repeat,
        .min_filter = .linear,
        .mag_filter = .linear,
        .mipmap_filter = .linear,
    }).?;
    defer linearRepeatSampler.release();

    const plainTexture = device.createTexture(&wgpu.TextureDescriptor{
        .label = "Texture",
        .usage = wgpu.TextureUsage.texture_binding | wgpu.TextureUsage.copy_dst,
        .format = .rgba8_unorm,
        .size = .{ .width = 4, .height = 4, .depth_or_array_layers = 1 },
    }).?;
    defer plainTexture.release();
    const plainTextureView = plainTexture.createView(null).?;
    defer plainTextureView.release();

    var texData: [4 * 4]@Vector(4, u8) = undefined;
    for (0..4) |y| {
        for (0..4) |x| {
            texData[y * 4 + x] = @splat(@truncate(255 * ((x + y) % 2)));
        }
    }
    queue.writeTexture(&wgpu.ImageCopyTexture{
        .texture = plainTexture,
        .origin = .{},
    }, &texData[0], std.mem.sliceAsBytes(&texData).len, &wgpu.TextureDataLayout{ .bytes_per_row = 4 * @sizeOf(std.meta.Elem(@TypeOf(texData))) }, &wgpu.Extent3D{ .width = 4, .height = 4, .depth_or_array_layers = 1 });

    const plainBindGroup = device.createBindGroup(&wgpu.BindGroupDescriptor{
        .label = "Plain",
        .layout = plainBindGroupLayout,
        .entry_count = 3,
        .entries = &[_]wgpu.BindGroupEntry{
            .{
                .binding = 0,
                .buffer = plainUniformsBuffer,
            },
            .{
                .binding = 1,
                .sampler = linearRepeatSampler,
            },
            .{
                .binding = 2,
                .texture_view = plainTextureView,
            },
        },
    }).?;
    defer plainBindGroup.release();

    var surfConfigured = false;
    const timeStart = std.time.microTimestamp();
    var frames: i64 = 0;
    while (!window.shouldClose()) {
        var surfaceTex: wgpu.SurfaceTexture = undefined;
        if (surfConfigured) {
            surface.getCurrentTexture(&surfaceTex);
        } else {
            surfaceTex.status = .outdated;
        }
        if (surfaceTex.suboptimal != 0 or surfaceTex.status == .outdated or surfaceTex.status == .lost) {
            const width, const height = window.getSize();
            surface.configure(&wgpu.SurfaceConfiguration{
                .device = device,
                .format = surfaceFormat,
                .width = @intCast(width),
                .height = @intCast(height),
                .present_mode = .immediate,
            });
            surfConfigured = true;
        } else if (surfaceTex.status != .success) {
            break;
        } else {
            {
                const timeNow = std.time.microTimestamp();
                const rot = zm.matFromAxisAngle(.{ 0, 0, 1, 0 }, @floatCast(@as(f64, @floatFromInt(timeNow - timeStart)) / 1e6));
                const width, const height = window.getSize();
                const wtoh = @as(f32, @floatFromInt(width)) / @as(f32, @floatFromInt(height));
                const ortho = zm.orthographicOffCenterLh(-1 * wtoh, 1 * wtoh, -1, 1, 0, 1);
                plainUniforms.worldViewProj = zm.mul(rot, ortho);
                queue.writeBuffer(plainUniformsBuffer, 0, &plainUniforms, @sizeOf(PlainUniforms));
            }

            const surfTexView = surfaceTex.texture.createView(null).?;
            defer surfTexView.release();

            const encoder = device.createCommandEncoder(&wgpu.CommandEncoderDescriptor{ .label = "Commands" }).?;

            const renderPass = encoder.beginRenderPass(&wgpu.RenderPassDescriptor{
                .label = "main",
                .color_attachment_count = 1,
                .color_attachments = &[_]wgpu.ColorAttachment{
                    .{
                        .view = surfTexView,
                        .load_op = .clear,
                        .store_op = .store,
                        .clear_value = wgpu.Color{ .r = 0.3, .g = 0.3, .b = 0.3, .a = 1.0 },
                    },
                },
            }).?;

            renderPass.setPipeline(plainPipeline);
            renderPass.setVertexBuffer(0, plainVerticesBuffer, 0, plainVerticesBuffer.getSize());
            renderPass.setBindGroup(0, plainBindGroup, 0, null);
            renderPass.draw(3, 1, 0, 0);

            renderPass.end();
            renderPass.release();

            const commands = encoder.finish(&wgpu.CommandBufferDescriptor{ .label = "main" }).?;
            encoder.release();

            queue.submit(&[_]*wgpu.CommandBuffer{commands});
            commands.release();

            surface.present();
            frames += 1;
        }

        _ = device.poll(false, null);
        glfw.pollEvents();
    }

    const timeNow = std.time.microTimestamp();
    const durationSecs: f64 = @as(f64, @floatFromInt(timeNow - timeStart)) / 1e6;
    std.debug.print("Frames: {d}, seconds: {d:.3}, FPS: {d:.3}\n", .{ frames, durationSecs, @as(f64, @floatFromInt(frames)) / durationSecs });
}

fn logCallback(level: wgpu.LogLevel, message: ?[*:0]const u8, userdata: ?*anyopaque) callconv(.C) void {
    _ = userdata;
    std.debug.print("Wgpu {?s}: {?s}\n", .{ std.enums.tagName(wgpu.LogLevel, level), message });
}

const builtin = @import("builtin");
const getSurfaceChain = switch (builtin.target.os.tag) {
    .windows => getSurfaceChainWin32,
    .linux => getSurfaceChainX11,
    else => unreachable,
};

fn getSurfaceChainX11(window: *glfw.Window) wgpu.SurfaceDescriptorFromXlibWindow {
    return wgpu.SurfaceDescriptorFromXlibWindow{
        .display = glfw.getX11Display().?,
        .window = glfw.getX11Window(window),
    };
}

fn getSurfaceChainWin32(window: *glfw.Window) wgpu.SurfaceDescriptorFromWindowsHWND {
    return wgpu.SurfaceDescriptorFromWindowsHWND{
        .hinstance = std.os.windows.kernel32.GetModuleHandleW(null).?,
        .hwnd = glfw.getWin32Window(window).?,
    };
}

fn getSurfaceFormat(surface: *wgpu.Surface, adapter: *wgpu.Adapter) ?wgpu.TextureFormat {
    var surfaceCaps: wgpu.SurfaceCapabilities = undefined;
    surface.getCapabilities(adapter, &surfaceCaps);
    defer surfaceCaps.FreeMembers();
    return if (surfaceCaps.format_count > 0) surfaceCaps.formats[0] else null;
}

fn createShaderModule(device: *wgpu.Device, filename: [*:0]const u8, alloc: std.mem.Allocator) !*wgpu.ShaderModule {
    const file = try std.fs.cwd().openFileZ(filename, .{});
    const content = try file.readToEndAllocOptions(alloc, 0xffffffff, null, @alignOf(u8), 0);
    defer alloc.free(content);
    return device.createShaderModule(&wgpu.ShaderModuleDescriptor{
        .next_in_chain = @ptrCast(&wgpu.ShaderModuleWGSLDescriptor{
            .code = content,
        }),
        .label = filename,
    }).?;
}
