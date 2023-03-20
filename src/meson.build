glo_inc = include_directories('.', '../include')
glo_lib = static_library(
    'glo',
    [
        'shader.cpp',
        'vao.cpp',
    ],
    include_directories: glo_inc,
    dependencies: glew_dep,
)
glo_dep = declare_dependency(
    link_with: glo_lib,
    include_directories: glo_inc,
    dependencies: glew_dep,
)
