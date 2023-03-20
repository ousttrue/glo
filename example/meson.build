glfw_dep = dependency('glfw3', default_options: ['install=true'])
glew_dep = dependency('glew')
executable(
    'example',
    'main.cpp',
    install: true,
    dependencies: [glfw_dep, glew_dep],
)
