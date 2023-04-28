cpp_compiler = meson.get_compiler('cpp')
user32_lib = cpp_compiler.find_library('user32', required: true)
d3d11_lib = cpp_compiler.find_library('d3d11', required: true)
d3dcompiler_lib = cpp_compiler.find_library('d3dcompiler', required: true)
executable(
    'dx11example',
    'main.cpp',
    install: true,
    gui_app: true,
    dependencies: [
        grapho_dep,
        user32_lib,
        d3d11_lib,
        d3dcompiler_lib,
    ],
)
