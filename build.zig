const std = @import("std");
const build_libs = @import("build_libs.zig");

const CFLAGS = [_][]const u8{
    "-std=c++20",
    "-D_USE_MATH_DEFINES=1",
    "-DUNICODE",
    "-D_UNICODE",
    "-DGLEW_STATIC",
    //
    // "-fms-extensions",
    // "-fms-compatibility",
    "-Wno-nonportable-include-path",
};

const Example = struct {
    name: []const u8,
    files: []const []const u8,
    using: build_libs.Using = .{},
};

const EXAMPLES = [_]Example{
    .{
        .name = "gl3",
        .files = &.{
            "example/gl3/main.cpp",
        },
        .using = .{
            .gl = true,
        },
    },
    .{
        .name = "camera",
        .files = &.{
            "example/glfw_platform/glfw_platform.cpp",
            "example/camera/main.cpp",
        },
        .using = .{
            .gl = true,
        },
    },
    .{
        .name = "pbr",
        .files = &.{
            "example/pbr/drawable.cpp",
            "example/pbr/imageloader.cpp",
            "example/pbr/main.cpp",
        },
        .using = .{
            .gl = true,
            .stb = true,
        },
    },
    .{
        .name = "normalmap",
        .files = &.{
            "example/normalmap/normalmap.cpp",
            "example/normalmap/scene.cpp",
            "example/normalmap/main.cpp",
        },
        .using = .{
            .gl = true,
            .stb = true,
            .glm = true,
        },
    },
    // .{
    //     .name = "dx11",
    //     .files = &.{
    //         "example/dx11/main.cpp",
    //     },
    //     .using = .{
    //         .directxmath = false,
    //         .dx = true,
    //     },
    // },
};

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    for (EXAMPLES) |example| {
        const exe = b.addExecutable(.{
            .name = example.name,
            .target = target,
            .optimize = optimize,
        });
        exe.linkLibCpp();
        for (example.files) |file| {
            exe.addCSourceFile(.{
                .file = b.path(file),
                .flags = &CFLAGS,
            });
        }

        build_libs.inject(b, exe, example.using, &CFLAGS);
    }
}
