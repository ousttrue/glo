glfw_dep = dependency(
    'glfw3',
    default_options: ['default_library=static'],
)
logl_dep = dependency('logl')
imgui_dep = dependency('imgui', default_options: ['default_library=static'])
glm_dep = dependency('glm')

subdir('glfw_platform')
subdir('gl3')
subdir('pbr')
subdir('normalmap')
subdir('camera')

if meson.get_compiler('cpp').get_id() == 'msvc'
    subdir('dx11')
endif
