gl_dep = dependency('gl')

glew_dep = dependency(
    'glew',
    default_options: ['default_library=static'],
)
directxmath_dep = dependency('directxmath')

grapho_inc = include_directories('.')

grapho_lib = static_library(
    'grapho',
    [
        'grapho/euclidean_transform.cpp',
        'grapho/vars.cpp',
        'grapho/camera/camera.cpp',
        'grapho/camera/ray.cpp',
        'grapho/gl3/vao.cpp',
        'grapho/gl3/texture.cpp',
        'grapho/gl3/shader.cpp',
        'grapho/gl3/cuberenderer.cpp',
        'grapho/gl3/fbo.cpp',
        'grapho/gl3/error_check.cpp',
    ],
    include_directories: grapho_inc,
    dependencies: [gl_dep, glew_dep, directxmath_dep],
)

grapho_dep = declare_dependency(
    include_directories: grapho_inc,
    link_with: grapho_lib,
    dependencies: [glew_dep, directxmath_dep],
)
