#include "glfw_platform.h"
#include <GL/glew.h>
#include <grapho/gl3/fbo.h>
#include <grapho/gl3/shader.h>
#include <grapho/gl3/texture.h>
#include <grapho/gl3/ubo.h>
#include <grapho/gl3/vao.h>
#include <iostream>
#include <stdio.h>
#include <string_view>

const auto WINDOW_NAME = "gl3window";
const auto WIDTH = 640;
const auto HEIGHT = 480;

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
static uint8_t indices[] = {
  0, 1, 2, //
  2, 3, 0, //
};
static rgba pixels[4] = {
  { 255, 0, 0, 255 },
  { 0, 255, 0, 255 },
  { 0, 0, 255, 255 },
  { 255, 255, 255, 255 },
};
struct MatrixData
{
  float mvp[16];
};

static const auto vertex_shader_text = u8R"(#version 400
layout (std140) uniform Scene { 
	mat4 mvp; 
} Mat;
in vec2 vPos;
in vec2 vUv;
out vec2 uv;
void main()
{
    gl_Position = Mat.mvp * vec4(vPos, 0.0, 1.0);
    uv = vUv;
};
)";

static const auto fragment_shader_text = u8R"(#version 400
in vec2 uv;
out vec4 FragColor;
uniform sampler2D colorTexture;

void main()
{
    FragColor = texture(colorTexture, uv);
};
)";

int
main(void)
{
  GlfwPlatform platform;
  if (!platform.CreateWindow(WINDOW_NAME, WIDTH, HEIGHT)) {
    return 1;
  }
  if (glewInit() != GLEW_OK) {
    return 2;
  }
  std::cout << "GL_VERSION: " << glGetString(GL_VERSION) << std::endl;
  std::cout << "GL_VENDOR: " << glGetString(GL_VENDOR) << std::endl;

  auto program = grapho::gl3::ShaderProgram::Create(vertex_shader_text,
                                                    fragment_shader_text);
  if (!program) {
    std::cerr << program.error() << std::endl;
    return 4;
  }

  auto vpos_location = (*program)->Attribute("vPos");
  if (!vpos_location) {
    return 6;
  }
  auto vuv_location = (*program)->Attribute("vUv");
  if (!vuv_location) {
    return 7;
  }

  auto ubo = grapho::gl3::Ubo::Create(sizeof(MatrixData), nullptr);
  auto block_index = (*program)->UboBlockIndex("Scene");
  if (!block_index) {
    return 8;
  }

  auto ibo =
    grapho::gl3::Ibo::Create(sizeof(indices), indices, GL_UNSIGNED_BYTE);
  auto vbo = grapho::gl3::Vbo::Create(sizeof(vertices), vertices);
  std::shared_ptr<grapho::gl3::Vbo> slots[] = {
    vbo,
  };
  grapho::VertexLayout layouts[] = {
    {
      .Id = { 
        .AttributeLocation=*vpos_location, 
        .Slot=0,
        .SemanticName="vPos", 
      },
      .Type = grapho::ValueType::Float,
      .Count = 2,
      .Offset = offsetof(Vertex, positon),
      .Stride = sizeof(Vertex),
    },
      {
      .Id = { 
        .AttributeLocation=*vuv_location,
        .Slot=0,
        .SemanticName="vUv", 
      },
      .Type = grapho::ValueType::Float,
      .Count = 2,
      .Offset = offsetof(Vertex, uv),
      .Stride = sizeof(Vertex),
    },
  };
  auto vao = grapho::gl3::Vao::Create(layouts, slots, ibo);

  auto texture = grapho::gl3::Texture::Create({
    2,
    2,
    grapho::PixelFormat::u8_RGBA,
    grapho::ColorSpace::Linear,
    &pixels[0].r,
  });
  texture->WrapClamp();
  texture->SamplingPoint();

  std::shared_ptr<grapho::gl3::Fbo> fbo = std::make_shared<grapho::gl3::Fbo>();
  std::shared_ptr<grapho::gl3::Texture> fboTexture =
    grapho::gl3::Texture::Create({
      512,
      512,
      grapho::PixelFormat::u8_RGBA,
    });

  MatrixData data
  {
    .mvp = {
      1, 0, 0, 0, //
      0, 1, 0, 0, //
      0, 0, 1, 0, //
      0, 0, 0, 1, //
    }, };
  const uint32_t ubo_binding_point = 1;
  grapho::camera::Viewport viewport;
  while (auto frame = platform.BeginFrame()) {
    // update
    viewport.Width = frame->Width;
    viewport.Height = frame->Height;
    ubo->Upload(data);
    (*program)->UboBind(*block_index, ubo_binding_point);
    ubo->SetBindingPoint(ubo_binding_point);

    {
      fbo->AttachTexture2D(fboTexture->Handle());
      grapho::camera::Viewport fboViewport{
        .Width = 512,
        .Height = 512,
        .Color = { 0, 0.2f, 0, 1.0f },
      };
      grapho::gl3::ClearViewport(fboViewport);
      (*program)->Use();

      texture->Activate(0);
      vao->Draw(GL_TRIANGLES, 6, 0);
      fbo->Unbind();
    }

    // draw backbuffer
    {
      (*program)->Use();
      fboTexture->Activate(0);
      grapho::gl3::ClearViewport(viewport);
      vao->Draw(GL_TRIANGLES, 6, 0);
    }

    // present
    platform.EndFrame();
  }

  return 0;
}
