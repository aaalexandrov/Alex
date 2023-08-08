const std = @import("std");

const glfw = @import("mach_glfw");
//const glfw = @import("libs/mach-glfw/build.zig");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "gfx",
        .root_source_file = .{ .path = "src/main.zig" },
        .target = target,
        .optimize = optimize,
    });
    b.installArtifact(exe);

    const glfw_dep = b.dependency("mach_glfw", .{
        .target = exe.target,
        .optimize = exe.optimize,
    });

    exe.addModule("glfw", glfw_dep.module("mach-glfw"));
    exe.linkLibrary(glfw_dep.artifact("mach-glfw"));
    // handle transitive dependencies of mach-glfw, until the package manager starts to be able to do that itself
    // in order for this to work, the .glfw and .vulkan_headers deps need to be pasted with the proper commits / hashes
    // to build.zig.zon from mach-glfw's build.zig.zon
    @import("glfw").addPaths(exe);
    exe.linkLibrary(b.dependency("vulkan_headers", .{
        .target = exe.target,
        .optimize = exe.optimize,
    }).artifact("vulkan-headers"));

    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
}
