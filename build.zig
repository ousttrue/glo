const std = @import("std");
const build_libs = @import("build_libs.zig");

const CFLAGS = [_][]const u8{
    "-std=c++20",
    "-D_USE_MATH_DEFINES=1",
    "-DUNICODE",
    "-D_UNICODE",
    //
    "-DGLEW_STATIC",
};

pub const Using = struct {
    // grapho
    directxmath: bool = true,
    imgui: bool = true,
    gl: bool = true,
    // glfw: bool = false,
    // glew: bool = false,
    // dx: bool = true,
    stb: bool = false,
    glm: bool = false,
};

const Example = struct {
    name: []const u8,
    files: []const []const u8,
    using: Using = .{},
};

const LIBS = [_][]const u8{
    "gdi32",
    "OpenGL32",
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

    const glew = build_libs.make_glew(b, target, optimize);
    const glfw = build_libs.make_glfw(b, target, optimize);

    // headeronly
    const directxmath = build_libs.make_directxmath(b);
    const stb = build_libs.make_stb(b);
    const glm = build_libs.make_glm(b);

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

        // build_libs.inject(b, exe, example.using, &CFLAGS);
        // grapho
        exe.addCSourceFiles(.{
            .files = &.{
                "src/grapho/vars.cpp",
                "src/grapho/camera/camera.cpp",
                "src/grapho/camera/ray.cpp",
            },
            .flags = &CFLAGS,
        });
        exe.addIncludePath(b.path("src"));

        exe.addIncludePath(b.path("example/glfw_platform"));
        exe.addCSourceFiles(.{
            .files = &.{
                "src/grapho/gl3/vao.cpp",
                "src/grapho/gl3/texture.cpp",
                "src/grapho/gl3/shader.cpp",
                "src/grapho/gl3/cuberenderer.cpp",
                "src/grapho/gl3/fbo.cpp",
                "src/grapho/gl3/error_check.cpp",
                "example/glfw_platform/glfw_platform.cpp",
            },
            .flags = &CFLAGS,
        });
        glew.injectIncludes(exe);
        if (glew.lib) |lib| {
            exe.linkLibrary(lib);
        }
        glfw.injectIncludes(exe);
        if (glfw.lib) |lib| {
            exe.linkLibrary(lib);
        }

        if (example.using.directxmath) {
            directxmath.injectIncludes(exe);
        }
        if (example.using.imgui) {
            build_libs.inject_imgui(b, exe);
        }
        if (example.using.stb) {
            stb.injectIncludes(exe);
        }
        if (example.using.glm) {
            glm.injectIncludes(exe);
        }
        for (LIBS) |lib| {
            exe.linkSystemLibrary(lib);
        }

        // install
        const install = b.addInstallArtifact(exe, .{});
        b.getInstallStep().dependOn(&install.step);
        // run
        const run_cmd = b.addRunArtifact(exe);
        run_cmd.step.dependOn(&install.step);
        if (b.args) |args| {
            run_cmd.addArgs(args);
        }
        const run_step = b.step(
            b.fmt("run-{s}", .{exe.name}),
            b.fmt("Run {s}", .{exe.name}),
        );
        run_step.dependOn(&run_cmd.step);
    }
}
