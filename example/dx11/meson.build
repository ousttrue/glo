cpp_compiler = meson.get_compiler('cpp')
user32_lib = cpp_compiler.find_library('user32', required: true)
executable(
    'dx11example',
    'main.cpp',
    install: true,
    gui_app: true,
    dependencies: [
        grapho_dep,
        user32_lib,
    ],
)
