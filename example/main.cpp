#include <gl/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdio.h>

static const struct
{
  float x, y;
  float r, g, b;
} vertices[3] = { { -0.6f, -0.4f, 1.f, 0.f, 0.f },
                  { 0.6f, -0.4f, 0.f, 1.f, 0.f },
                  { 0.f, 0.6f, 0.f, 0.f, 1.f } };

static const char* vertex_shader_text = R"(#version 110
uniform mat4 MVP;
attribute vec3 vCol;
attribute vec2 vPos;
varying vec3 color;
void main()
{
    gl_Position = MVP * vec4(vPos, 0.0, 1.0);
    color = vCol;
};
)";

static const char* fragment_shader_text = R"(#version 110
varying vec3 color;
void main()
{
    gl_FragColor = vec4(color, 1.0);
};
)";

static void
error_callback(int error, const char* description)
{
  fprintf(stderr, "Error: %s\n", description);
}

static void
key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int
main(void)
{
  GLFWwindow* window;
  GLuint vertex_buffer, vertex_shader, fragment_shader, program;
  GLint mvp_location, vpos_location, vcol_location;

  glfwSetErrorCallback(error_callback);

  if (!glfwInit()) {
    return 1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
  if (!window) {
    glfwTerminate();
    return 2;
  }

  glfwSetKeyCallback(window, key_callback);

  glfwMakeContextCurrent(window);
  if (glewInit() != GLEW_OK) {
    return 3;
  }
  std::cout << "GL_VERSION: " << glGetString(GL_VERSION) << std::endl;
  std::cout << "GL_VENDOR: " << glGetString(GL_VENDOR) << std::endl;
  glfwSwapInterval(1);

  // NOTE: OpenGL error checks have been omitted for brevity

  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
  glCompileShader(vertex_shader);

  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
  glCompileShader(fragment_shader);

  program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);

  mvp_location = glGetUniformLocation(program, "MVP");
  vpos_location = glGetAttribLocation(program, "vPos");
  vcol_location = glGetAttribLocation(program, "vCol");

  glEnableVertexAttribArray(vpos_location);
  glVertexAttribPointer(
    vpos_location, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void*)0);
  glEnableVertexAttribArray(vcol_location);
  glVertexAttribPointer(vcol_location,
                        3,
                        GL_FLOAT,
                        GL_FALSE,
                        sizeof(vertices[0]),
                        (void*)(sizeof(float) * 2));

  while (!glfwWindowShouldClose(window)) {
    float mvp[16] = {
      1, 0, 0, 0, //
      0, 1, 0, 0, //
      0, 0, 1, 0, //
      0, 0, 0, 1, //
    };

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program);
    glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);

  glfwTerminate();
  return 0;
}
