project(
    'grapho',
    'cpp',
    version: '1.6.0',
    license: 'MIT',
)

grapho_inc = include_directories('include')
grapho_dep = declare_dependency(
    include_directories: grapho_inc,
)

if get_option('example')
    subdir('example')
endif
