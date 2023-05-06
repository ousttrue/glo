project('grapho', 'cpp')

grapho_inc = include_directories('include')
grapho_dep = declare_dependency(
    include_directories: grapho_inc,
)

if get_option('example')
    glew_dep = dependency(
        'glew',
        default_options: ['default_library=static'],
    )
    glfw_dep = dependency(
        'glfw3',
        default_options: ['default_library=static'],
    )
    logl_dep = dependency('logl')
    imgui_dep = dependency('imgui')

    subdir('example')
endif
