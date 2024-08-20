const std = @import("std");
const build_libs = @import("build_libs.zig");
pub const CLib = build_libs.CLib;
pub const make_glfw = build_libs.make_glfw;
pub const make_glew = build_libs.make_glew;
pub const make_imgui = build_libs.make_imgui;
pub const make_grapho = build_libs.make_grapho;

pub const Using = struct {
    directxmath: bool = true,
    imgui: bool = true,
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
    },
    .{
        .name = "camera",
        .files = &.{
            "example/camera/main.cpp",
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

    const glew = make_glew(b, b.dependency("glew", .{}), target, optimize);
    const glfw = make_glfw(b, b.dependency("glfw", .{}), target, optimize);
    const imgui = make_imgui(b, b.dependency("imgui", .{}), target, optimize);
    const grapho = make_grapho(b, b.dependency("grapho", .{}), target, optimize);

    // headeronly
    const directxmath = CLib.make_headeronly(b.dependency("directxmath", .{}), &.{"Inc"});
    const stb = CLib.make_headeronly(b.dependency("stb", .{}), &.{""});
    const glm = CLib.make_headeronly(b.dependency("glm", .{}), &.{""});

    // inject libs
    if (grapho.lib) |lib| {
        directxmath.injectIncludes(lib);
        glfw.injectIncludes(lib);
        glew.injectIncludes(lib);
    }
    if (imgui.lib) |lib| {
        glfw.injectIncludes(lib);
    }

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
                .flags = grapho.flags,
            });
        }
        exe.addCSourceFile(.{
            .file = b.path("example/glfw_platform/glfw_platform.cpp"),
            .flags = grapho.flags,
        });
        exe.addIncludePath(b.path("example/glfw_platform"));
        glfw.injectIncludes(exe);

        // libs inject to exe
        grapho.injectIncludes(exe);
        if (grapho.lib) |lib| {
            exe.linkLibrary(lib);
        }
        glew.injectIncludes(exe);
        if (glew.lib) |lib| {
            exe.linkLibrary(lib);
        }
        if (glfw.lib) |lib| {
            exe.linkLibrary(lib);
        }

        if (example.using.directxmath) {
            directxmath.injectIncludes(exe);
        }
        if (example.using.imgui) {
            imgui.injectIncludes(exe);
            if (imgui.lib) |lib| {
                exe.linkLibrary(lib);
            }
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
