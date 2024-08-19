project(
    'grapho',
    'cpp',
    version: '1.11.0',
    license: 'MIT',
    # for msvc
    default_options: ['cpp_std=c++20'],
)

subdir('src')

if get_option('example')
    subdir('example')
endif
