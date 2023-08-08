const std = @import("std");
const glfw = @import("glfw");
const vk = @import("vk.zig");
const rhi = @import("rhi.zig");

const app_name = "gfx";

fn setAll(comptime T: type, v: anytype) T {
    const fields = std.meta.fields(T);

    var result: T = undefined;
    inline for (fields) |field| {
        @field(result, field.name) = if (comptime field.type == @TypeOf(v))
            v
        else
            std.mem.zeroes(field.type);
    }
    return result;
}

pub fn main() !void {
    glfw.setErrorCallback(errorCallback);

    if (!glfw.init(.{})) {
        std.log.err("Failed to initialize glfw\n", .{});
        std.process.exit(1);
    }
    defer glfw.terminate();

    var window = glfw.Window.create(1024, 768, app_name, null, null, .{
        .client_api = .no_api,
    }) orelse {
        std.log.err("Failed to create window\n", .{});
        std.process.exit(1);
    };
    defer window.destroy();

    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    const allocator = gpa.allocator();

    var Vk = try rhi.Vk.init(app_name, window, allocator);
    defer Vk.deinit();

    while (!window.shouldClose()) {
        glfw.pollEvents();
    }
}

fn errorCallback(error_code: glfw.ErrorCode, description: [:0]const u8) void {
    std.log.err("glfw: {}: {s}\n", .{ error_code, description });
}
