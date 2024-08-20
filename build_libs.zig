const std = @import("std");

pub const CLib = struct {
    dep: *std.Build.Dependency,
    lib: ?*std.Build.Step.Compile = null,
    includes: []const []const u8 = &.{},
    flags: []const []const u8 = &.{},

    pub fn make_headeronly(dep: *std.Build.Dependency, includes: []const []const u8) CLib {
        return .{
            .dep = dep,
            .includes = includes,
        };
    }

    pub fn injectIncludes(self: @This(), compile: *std.Build.Step.Compile) void {
        for (self.includes) |include| {
            compile.addIncludePath(self.dep.path(include));
        }
    }
};

pub fn make_glfw(
    b: *std.Build,
    glfw_dep: *std.Build.Dependency,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
) CLib {
    const GLFW_FILES = [_][]const u8{
        "context.c",
        "init.c",
        "input.c",
        "monitor.c",
        "platform.c",
        "vulkan.c",
        "window.c",
        "egl_context.c",
        "osmesa_context.c",
        "null_init.c",
        "null_monitor.c",
        "null_window.c",
        "null_joystick.c",
    };

    const GLFW_FILES_WIN32 = [_][]const u8{
        "win32_module.c",
        "win32_time.c",
        "win32_thread.c",
        "win32_init.c",
        "win32_joystick.c",
        "win32_monitor.c",
        "win32_window.c",
        "wgl_context.c",
    };

    var lib = b.addStaticLibrary(.{
        .name = "glfw",
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    lib.addIncludePath(glfw_dep.path("include"));
    lib.addCSourceFiles(.{
        .root = glfw_dep.path("src"),
        .files = &(GLFW_FILES ++ GLFW_FILES_WIN32),
        .flags = &.{"-D_GLFW_WIN32"},
    });
    return .{
        .dep = glfw_dep,
        .lib = lib,
        .includes = &.{
            "include",
        },
    };
}

pub fn make_glew(
    b: *std.Build,
    glew_dep: *std.Build.Dependency,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
) CLib {
    const GLEW_FILES = [_][]const u8{
        "glew.c",
    };
    var lib = b.addStaticLibrary(.{
        .name = "glew",
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    lib.addIncludePath(glew_dep.path("include"));
    lib.addCSourceFiles(.{
        .root = glew_dep.path("src"),
        .files = &GLEW_FILES,
        .flags = &.{"-DGLEW_STATIC"},
    });
    return .{
        .dep = glew_dep,
        .lib = lib,
        .includes = &.{"include"},
        .flags = &.{"-DGLEW_STATIC"},
    };
}

pub fn make_imgui(
    b: *std.Build,
    imgui_dep: *std.Build.Dependency,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
) CLib {
    const IMGUI_FILES = [_][]const u8{
        "imgui.cpp",
        "imgui_draw.cpp",
        "imgui_tables.cpp",
        "imgui_widgets.cpp",
        "imgui_demo.cpp",
        //
        "backends/imgui_impl_glfw.cpp",
        "backends/imgui_impl_opengl3.cpp",
    };
    // const imgui_dep = b.dependency("imgui", .{});
    var lib = b.addStaticLibrary(.{
        .name = "imgui",
        .target = target,
        .optimize = optimize,
    });
    lib.linkLibCpp();
    lib.addIncludePath(imgui_dep.path(""));
    lib.addIncludePath(imgui_dep.path("backends"));
    lib.addCSourceFiles(.{
        .root = imgui_dep.path(""),
        .files = &IMGUI_FILES,
    });
    return .{
        .dep = imgui_dep,
        .lib = lib,
        .includes = &.{
            "", "backends",
        },
    };
}

pub fn make_grapho(
    b: *std.Build,
    dep: *std.Build.Dependency,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
) CLib {
    const CFLAGS = [_][]const u8{
        "-std=c++20",
        "-D_USE_MATH_DEFINES=1",
        "-DUNICODE",
        "-D_UNICODE",
        //
        "-DGLEW_STATIC",
    };

    var lib = b.addStaticLibrary(.{
        .name = "grapho",
        .target = target,
        .optimize = optimize,
    });
    lib.linkLibCpp();
    lib.addCSourceFiles(.{
        .root = dep.path(""),
        .files = &.{
            "grapho/vars.cpp",
            "grapho/camera/camera.cpp",
            "grapho/camera/ray.cpp",
        },
        .flags = &CFLAGS,
    });
    lib.addIncludePath(dep.path(""));
    lib.addCSourceFiles(.{
        .root = dep.path(""),
        .files = &.{
            "grapho/gl3/vao.cpp",
            "grapho/gl3/texture.cpp",
            "grapho/gl3/shader.cpp",
            "grapho/gl3/cuberenderer.cpp",
            "grapho/gl3/fbo.cpp",
            "grapho/gl3/error_check.cpp",
        },
        .flags = &CFLAGS,
    });
    return .{
        .dep = dep,
        .lib = lib,
        .includes = &.{""},
        .flags = &CFLAGS,
    };
}
