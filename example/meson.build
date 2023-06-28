glew_dep = dependency(
    'glew',
    default_options: ['default_library=static'],
)
glfw_dep = dependency(
    'glfw3',
    default_options: ['default_library=static'],
)
logl_dep = dependency('logl')
imgui_dep = dependency('imgui', default_options: ['default_library=static'])
directxmath_dep = dependency('directxmath', include_type: 'system')
glm_dep = dependency('glm')

if host_machine.system() == 'windows'
    subdir('dx11')
endif

subdir('glfw_platform')
subdir('gl3')
subdir('pbr')
subdir('normalmap')
subdir('camera')
