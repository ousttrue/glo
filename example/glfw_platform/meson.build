platform_inc = include_directories('.')
platform_lib = static_library(
    'glfwplatform',
    ['glfw_platform.cpp'],
    include_directories: platform_inc,
    dependencies: [glfw_dep],
)
platform_dep = declare_dependency(
    link_with: platform_lib,
    include_directories: platform_inc,
)
