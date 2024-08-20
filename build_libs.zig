const std = @import("std");

pub const CLib = struct {
    dep: *std.Build.Dependency,
    lib: ?*std.Build.Step.Compile = null,
    includes: []const []const u8 = &.{},
    flags: []const []const u8 = &.{},

    pub fn injectIncludes(self: @This(), compile: *std.Build.Step.Compile) void {
        for (self.includes) |include| {
            compile.addIncludePath(self.dep.path(include));
        }
    }
};

pub fn make_glfw(
    b: *std.Build,
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

    const glfw_dep = b.dependency("glfw", .{});
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
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
) CLib {
    const glew_dep = b.dependency("glew", .{});
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

pub fn inject_imgui(b: *std.Build, exe: *std.Build.Step.Compile) void {
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
    const imgui_dep = b.dependency("imgui", .{});
    exe.addIncludePath(imgui_dep.path(""));
    exe.addIncludePath(imgui_dep.path("backends"));
    exe.addCSourceFiles(.{
        .root = imgui_dep.path(""),
        .files = &IMGUI_FILES,
    });
}

pub fn make_directxmath(b: *std.Build) CLib {
    const directxmath_dep = b.dependency("directxmath", .{});
    return .{
        .dep = directxmath_dep,
        .includes = &.{"Inc"},
    };
}

pub fn make_stb(b: *std.Build) CLib {
    const stb_dep = b.dependency("stb", .{});
    return .{
        .dep = stb_dep,
        .includes = &.{""},
    };
}

pub fn make_glm(b: *std.Build) CLib {
    const glm_dep = b.dependency("glm", .{});
    return .{
        .dep = glm_dep,
        .includes = &.{""},
    };
}
