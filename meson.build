project('grapho', 'cpp', default_options: ['cpp_std=c++latest'])

grapho_inc = include_directories('include')
grapho_dep = declare_dependency(
    include_directories: grapho_inc,
)

glew_dep = dependency('glew')
glfw_dep = dependency('glfw3', default_options: ['install=true'])
subdir('example/gl3')
