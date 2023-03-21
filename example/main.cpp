#include <gl/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glo/shader.h>
#include <glo/vao.h>
#include <iostream>
#include <stdio.h>
#include <string_view>

struct float2
{
  float x, y;
};
struct float3
{
  float x, y, z;
};
struct Vertex
{
  float2 positon;
  float3 rgb;
};
static const struct Vertex vertices[3] = {
  { { -0.6f, -0.4f }, { 1.f, 0.f, 0.f } },
  { { 0.6f, -0.4f }, { 0.f, 1.f, 0.f } },
  { { 0.f, 0.6f }, { 0.f, 0.f, 1.f } }
};

static const char* vertex_shader_text = R"(#version 400
uniform mat4 MVP;
in vec3 vCol;
in vec2 vPos;
out vec3 color;
void main()
{
    gl_Position = MVP * vec4(vPos, 0.0, 1.0);
    color = vCol;
};
)";

static const char* fragment_shader_text = R"(#version 400
in vec3 color;
out vec4 FragColor;
void main()
{
    FragColor = vec4(color, 1.0);
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

static void
printError(std::string_view msg)
{
  std::cout << msg << std::endl;
}

int
main(void)
{

  glfwSetErrorCallback(error_callback);

  if (!glfwInit()) {
    return 1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  auto window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
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

  auto program =
    *glo::ShaderProgram::Create(vertex_shader_text, fragment_shader_text);
  auto mvp_location = *program->UniformLocation("MVP");
  auto vpos_location = *program->AttributeLocation("vPos");
  auto vcol_location = *program->AttributeLocation("vCol");

  auto vbo = glo::Vbo::Create(sizeof(vertices), vertices);
  glo::VertexLayout layouts[] = {
    {
      .id = { "vCol", vcol_location },
      .type = glo::ValueType::Float,
      .count = 3,
      .offset = offsetof(Vertex, rgb),
      .stride = sizeof(Vertex),
    },
    {
      .id = { "vPos", vpos_location },
      .type = glo::ValueType::Float,
      .count = 2,
      .offset = offsetof(Vertex, positon),
      .stride = sizeof(Vertex),
    },
  };
  glo::VertexSlot slots[] = {
    {
      .location = vcol_location,
      .vbo = vbo,
    },
    {
      .location = vpos_location,
      .vbo = vbo,
    },
  };
  auto vao = glo::Vao::Create(layouts, slots);

  while (!glfwWindowShouldClose(window)) {
    // update
    glfwPollEvents();
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

    // draw
    program->Bind();
    program->SetUniformMatrix(mvp_location, mvp);
    vao->Draw(GL_TRIANGLES, 3, 0);

    // present
    glfwSwapBuffers(window);
  }

  glfwDestroyWindow(window);

  glfwTerminate();
  return 0;
}
