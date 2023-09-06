project(
    'grapho',
    'cpp',
    version: '1.8.0',
    license: 'MIT',
)

gl_dep = dependency('gl')

grapho_inc = include_directories('src')

grapho_lib = static_library(
    'grapho',
    [
        'src/grapho/camera/camera.cpp',
        'src/grapho/euclidean_transform.cpp',
        'src/grapho/gl3/cuberenderer.cpp',
        'src/grapho/gl3/texture.cpp',
    ],
    include_directories: grapho_inc,
    dependencies: [gl_dep],
)

grapho_dep = declare_dependency(
    include_directories: grapho_inc,
    link_with: grapho_lib,
)

if get_option('example')
    subdir('example')
endif
