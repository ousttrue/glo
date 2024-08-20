const std = @import("std");

const LIBS = [_][]const u8{
    "gdi32",
    "OpenGL32",
};

const WINDOWS_KITS_INCLUDES = [_][]const u8{
    "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/cppwinrt",
    "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/um",
    "C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/shared",
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

pub fn inject_glfw(b: *std.Build, exe: *std.Build.Step.Compile) void {
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
    exe.addIncludePath(glfw_dep.path("include"));
    exe.addCSourceFiles(.{
        .root = glfw_dep.path("src"),
        .files = &(GLFW_FILES ++ GLFW_FILES_WIN32),
        .flags = &.{"-D_GLFW_WIN32"},
    });
}

pub fn inject_glew(b: *std.Build, exe: *std.Build.Step.Compile) void {
    const GLEW_FILES = [_][]const u8{
        "glew.c",
    };

    const glew_dep = b.dependency("glew", .{});
    exe.addIncludePath(glew_dep.path("include"));
    exe.addCSourceFiles(.{
        .root = glew_dep.path("src"),
        .files = &GLEW_FILES,
    });
}

pub fn inject_directxmath(b: *std.Build, exe: *std.Build.Step.Compile) void {
    const directxmath_dep = b.dependency("directxmath", .{});
    exe.addIncludePath(directxmath_dep.path("Inc"));
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

pub fn inject_stb(b: *std.Build, exe: *std.Build.Step.Compile) void {
    const directxmath_dep = b.dependency("stb", .{});
    exe.addIncludePath(directxmath_dep.path(""));
}

pub fn inject_glm(b: *std.Build, exe: *std.Build.Step.Compile) void {
    const glm_dep = b.dependency("glm", .{});
    exe.addIncludePath(glm_dep.path(""));
}

pub fn inject(
    b: *std.Build,
    exe: *std.Build.Step.Compile,
    using: Using,
    cflags: []const []const u8,
) void {
    // grapho
    exe.addCSourceFiles(.{
        .files = &.{
            "src/grapho/vars.cpp",
            "src/grapho/camera/camera.cpp",
            "src/grapho/camera/ray.cpp",
        },
        .flags = cflags,
    });
    exe.addIncludePath(b.path("src"));

    if (using.gl) {
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
            .flags = cflags,
        });
        inject_glew(b, exe);
        inject_glfw(b, exe);
    }

    if (using.directxmath) {
        inject_directxmath(b, exe);
    }
    if (using.imgui) {
        inject_imgui(b, exe);
    }
    if (using.stb) {
        inject_stb(b, exe);
    }
    if (using.glm) {
        inject_glm(b, exe);
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
