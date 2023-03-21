project('glo', 'cpp', default_options: ['cpp_std=c++latest'])

glo_inc = include_directories('include')
glo_dep = declare_dependency(
    include_directories: glo_inc,
)

glew_dep = dependency('glew')
subdir('example')
