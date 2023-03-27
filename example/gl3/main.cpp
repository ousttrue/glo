#include <gl/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <grapho/gl3/fbo.h>
#include <grapho/gl3/shader.h>
#include <grapho/gl3/texture.h>
#include <grapho/gl3/vao.h>
#include <iostream>
#include <stdio.h>
#include <string_view>

struct rgba
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
};

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
  float2 uv;
};
auto s = 0.5f;
/// CCW
/// 3   2
/// +---+
/// |   |
/// +---+
/// 0   1
static const struct Vertex vertices[] = {
  { { -s, -s }, { 0.f, 1.f } },
  { { s, -s }, { 1.f, 1.f } },
  { { s, s }, { 1.f, 0.f } },
  { { -s, s }, { 0.f, 0.f } },
};

static const char* vertex_shader_text = R"(#version 400
uniform mat4 MVP;
in vec2 vPos;
in vec2 vUv;
out vec2 uv;
void main()
{
    gl_Position = MVP * vec4(vPos, 0.0, 1.0);
    uv = vUv;
};
)";

static const char* fragment_shader_text = R"(#version 400
in vec2 uv;
out vec4 FragColor;
uniform sampler2D colorTexture;

void main()
{
    FragColor = texture(colorTexture, uv);
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
  namespace glo = grapho::gl3;

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
  auto vuv_location = *program->AttributeLocation("vUv");

  static uint8_t indices[] = {
    0, 1, 2, 2, 3, 0,
  };
  auto ibo = glo::Ibo::Create(sizeof(indices), indices, GL_UNSIGNED_BYTE);
  auto vbo = glo::Vbo::Create(sizeof(vertices), vertices);
  glo::VertexLayout layouts[] = {
    {
      .id = { "vPos", vpos_location },
      .type = glo::ValueType::Float,
      .count = 2,
      .offset = offsetof(Vertex, positon),
      .stride = sizeof(Vertex),
    },
    {
      .id = { "vUv", vuv_location },
      .type = glo::ValueType::Float,
      .count = 2,
      .offset = offsetof(Vertex, uv),
      .stride = sizeof(Vertex),
    },
  };
  glo::VertexSlot slots[] = {
    {
      .location = vpos_location,
      .vbo = vbo,
    },
    {
      .location = vuv_location,
      .vbo = vbo,
    },
  };
  auto vao = glo::Vao::Create(layouts, slots, ibo);

  static rgba pixels[4] = {
    { 255, 0, 0, 255 },
    { 0, 255, 0, 255 },
    { 0, 0, 255, 255 },
    { 255, 255, 255, 255 },
  };

  auto texture = glo::Texture::Create(2, 2, &pixels[0].r);

  std::shared_ptr<glo::Fbo> fbo;

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

    // draw fbo
    if (!fbo) {
      fbo = glo::Fbo::Create(512, 512);
    }
    {
      fbo->Bind();
      glViewport(0, 0, 512, 512);
      glClearColor(0, 0.2f, 0, 0);
      glClear(GL_COLOR_BUFFER_BIT);
      program->Bind();
      program->SetUniformMatrix(mvp_location, mvp);
      texture->Bind(0);
      vao->Draw(GL_TRIANGLES, 6, 0);
      fbo->Unbind();
    }

    // draw backbuffer
    {
      glfwGetFramebufferSize(window, &width, &height);
      glViewport(0, 0, width, height);
      glClearColor(0, 0, 0, 0);
      glClear(GL_COLOR_BUFFER_BIT);
      program->Bind();
      program->SetUniformMatrix(mvp_location, mvp);
      fbo->texture->Bind(0);
      vao->Draw(GL_TRIANGLES, 6, 0);
    }

    // present
    glfwSwapBuffers(window);
  }

  glfwDestroyWindow(window);

  glfwTerminate();
  return 0;
}
