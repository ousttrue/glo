#include "renderer.h"
#include "env.h"
#include "mesh.h"
#include "pbrmaterial.h"
#include <learnopengl/filesystem.h>
#include <stdexcept>

static DirectX::XMFLOAT3X3
TransposeInv(const DirectX::XMFLOAT4X4& _m)
{
  auto m = DirectX::XMLoadFloat4x4(&_m);
  DirectX::XMVECTOR det;
  auto ti = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&det, m));
  DirectX::XMFLOAT3X3 mat3;
  DirectX::XMStoreFloat3x3(&mat3, ti);
  return mat3;
}

struct Light
{
  DirectX::XMFLOAT3 Position;
  DirectX::XMFLOAT3 Color;
};

struct Drawable
{
  std::shared_ptr<grapho::gl3::Vao> Mesh;
  uint32_t MeshDrawCount = 0;
  std::shared_ptr<grapho::gl3::ShaderProgram> Shader;
  std::shared_ptr<PbrMaterial> Material;
  DirectX::XMFLOAT3 Position = {};

  Drawable()
  {
    {
      auto shader = grapho::gl3::ShaderProgram::CreateFromPath("2.2.2.pbr.vs",
                                                               "2.2.2.pbr.fs");
      if (!shader) {
        throw std::runtime_error(shader.error());
      }
      Shader = *shader;
      Shader->Use();
      Shader->Uniform("irradianceMap")->SetInt(0);
      Shader->Uniform("prefilterMap")->SetInt(1);
      Shader->Uniform("brdfLUT")->SetInt(2);
      Shader->Uniform("albedoMap")->SetInt(3);
      Shader->Uniform("normalMap")->SetInt(4);
      Shader->Uniform("metallicMap")->SetInt(5);
      Shader->Uniform("roughnessMap")->SetInt(6);
      Shader->Uniform("aoMap")->SetInt(7);
    }
    {
      auto sphere = Mesh::Sphere();

      auto vbo = grapho::gl3::Vbo::Create(sphere->Vertices);
      std::shared_ptr<grapho::gl3::Vbo> slots[]{
        vbo,
      };
      auto ibo = grapho::gl3::Ibo::Create(sphere->Indices);

      Mesh = grapho::gl3::Vao::Create(sphere->Layouts, slots, ibo);
      MeshDrawCount = sphere->Indices.size();
    }
  }

  ~Drawable() {}

  void Draw(const DirectX::XMFLOAT4X4& projection,
            const DirectX::XMFLOAT4X4& view,
            const DirectX::XMFLOAT3& cameraPos,
            std::span<const std::shared_ptr<Light>> lights)
  {
    for (unsigned int i = 0; i < lights.size(); ++i) {
      // glm::vec3 newPos =
      //   lightPositions[i] + glm::vec3(sin(currentFrame * 5.0) * 5.0, 0.0,
      //   0.0);
      auto newPos = lights[i]->Position;
      Shader->Uniform("lightPositions[" + std::to_string(i) + "]")
        ->SetFloat3(newPos);
      Shader->Uniform("lightColors[" + std::to_string(i) + "]")
        ->SetFloat3(lights[i]->Color);

      // model = glm::mat4(1.0f);
      // model = glm::translate(model, newPos);
      // model = glm::scale(model, glm::vec3(0.5f));
      // PbrShader->Uniform("model")->SetMat4(model);
      // PbrShader->Uniform("normalMatrix")
      //   ->SetMat3(glm::transpose(glm::inverse(glm::mat3(model))));
      // Sphere->Draw(GL_TRIANGLE_STRIP, SphereDrawCount);
    }

    Shader->Use();
    // auto view = cameraViewMatrix();
    Shader->Uniform("view")->SetMat4(view);
    // auto cameraPos = cameraPosition();
    Shader->Uniform("camPos")->SetFloat3(cameraPos);
    // initialize static shader uniforms before rendering
    Shader->Uniform("projection")->SetMat4(projection);

    DirectX::XMFLOAT4X4 model;
    DirectX::XMStoreFloat4x4(
      &model, DirectX::XMMatrixTranslation(Position.x, Position.y, Position.z));
    Shader->Uniform("model")->SetMat4(model);
    Shader->Uniform("normalMatrix")->SetMat3(TransposeInv(model));
    Material->Bind();
    Mesh->Draw(GL_TRIANGLE_STRIP, MeshDrawCount);
  }
};

Renderer::Renderer()
{
  // configure global opengl state
  // -----------------------------
  glEnable(GL_DEPTH_TEST);
  // set depth function to less than AND equal for skybox depth trick.
  glDepthFunc(GL_LEQUAL);
  // enable seamless cubemap sampling for lower mip levels in the pre-filter
  // map.
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

  auto iron = std::make_shared<Drawable>();
  iron->Material = PbrMaterial::Create(
    FileSystem::getPath("resources/textures/pbr/rusted_iron/albedo.png"),
    FileSystem::getPath("resources/textures/pbr/rusted_iron/normal.png"),
    FileSystem::getPath("resources/textures/pbr/rusted_iron/metallic.png"),
    FileSystem::getPath("resources/textures/pbr/rusted_iron/roughness.png"),
    FileSystem::getPath("resources/textures/pbr/rusted_iron/ao.png"));
  iron->Position = { -5.0, 0.0, 2.0 };
  Drawables.push_back(iron);

  auto gold = std::make_shared<Drawable>();
  gold->Material = PbrMaterial::Create(
    FileSystem::getPath("resources/textures/pbr/gold/albedo.png"),
    FileSystem::getPath("resources/textures/pbr/gold/normal.png"),
    FileSystem::getPath("resources/textures/pbr/gold/metallic.png"),
    FileSystem::getPath("resources/textures/pbr/gold/roughness.png"),
    FileSystem::getPath("resources/textures/pbr/gold/ao.png"));
  gold->Position = { -3.0, 0.0, 2.0 };
  Drawables.push_back(gold);

  auto grass = std::make_shared<Drawable>();
  grass->Material = PbrMaterial::Create(
    FileSystem::getPath("resources/textures/pbr/grass/albedo.png"),
    FileSystem::getPath("resources/textures/pbr/grass/normal.png"),
    FileSystem::getPath("resources/textures/pbr/grass/metallic.png"),
    FileSystem::getPath("resources/textures/pbr/grass/roughness.png"),
    FileSystem::getPath("resources/textures/pbr/grass/ao.png"));
  grass->Position = { -1.0, 0.0, 2.0 };
  Drawables.push_back(grass);

  auto plastic = std::make_shared<Drawable>();
  plastic->Material = PbrMaterial::Create(
    FileSystem::getPath("resources/textures/pbr/plastic/albedo.png"),
    FileSystem::getPath("resources/textures/pbr/plastic/normal.png"),
    FileSystem::getPath("resources/textures/pbr/plastic/metallic.png"),
    FileSystem::getPath("resources/textures/pbr/plastic/roughness.png"),
    FileSystem::getPath("resources/textures/pbr/plastic/ao.png"));
  plastic->Position = { 1.0, 0.0, 2.0 };
  Drawables.push_back(plastic);

  auto wall = std::make_shared<Drawable>();
  wall->Material = PbrMaterial::Create(
    FileSystem::getPath("resources/textures/pbr/wall/albedo.png"),
    FileSystem::getPath("resources/textures/pbr/wall/normal.png"),
    FileSystem::getPath("resources/textures/pbr/wall/metallic.png"),
    FileSystem::getPath("resources/textures/pbr/wall/roughness.png"),
    FileSystem::getPath("resources/textures/pbr/wall/ao.png"));
  wall->Position = { 3.0, 0.0, 2.0 };
  Drawables.push_back(wall);

  {
    auto light = std::make_shared<Light>();
    light->Position = { -10.0f, 10.0f, 10.0f };
    light->Color = { 300.0f, 300.0f, 300.0f };
    Lights.push_back(light);
  }
  {
    auto light = std::make_shared<Light>();
    light->Position = { 10.0f, 10.0f, 10.0f };
    light->Color = { 300.0f, 300.0f, 300.0f };
    Lights.push_back(light);
  }
  {
    auto light = std::make_shared<Light>();
    light->Position = { -10.0f, -10.0f, 10.0f };
    light->Color = { 300.0f, 300.0f, 300.0f };
    Lights.push_back(light);
  }
  {
    auto light = std::make_shared<Light>();
    light->Position = { 10.0f, -10.0f, 10.0f };
    light->Color = { 300.0f, 300.0f, 300.0f };
    Lights.push_back(light);
  }

  Env = std::make_shared<Environment>();
}

Renderer::~Renderer() {}

void
Renderer::Render(int scrWidth,
                 int scrHeight,
                 const DirectX::XMFLOAT4X4& projection,
                 const DirectX::XMFLOAT4X4& view,
                 const DirectX::XMFLOAT3& cameraPos)
{
  glViewport(0, 0, scrWidth, scrHeight);

  // per-frame time logic
  // --------------------
  // deltaTime = currentFrame - lastFrame;
  // lastFrame = currentFrame;

  // render
  // ------
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // bind pre-computed IBL data
  Env->Bind();

  for (auto& drawable : Drawables) {
    drawable->Draw(projection, view, cameraPos, Lights);
  }

  // render light source (simply re-render sphere at light positions)
  // this looks a bit off as we use the same shader, but it'll make their
  // positions obvious and keeps the codeprint small.
  // for (unsigned int i = 0;
  //      i < sizeof(lightPositions) / sizeof(lightPositions[0]);
  //      ++i) {
  //   glm::vec3 newPos =
  //     lightPositions[i] + glm::vec3(sin(currentFrame * 5.0) * 5.0, 0.0, 0.0);
  //   newPos = lightPositions[i];
  //   PbrShader->Uniform("lightPositions[" + std::to_string(i) + "]")
  //     ->SetFloat3(newPos);
  //   PbrShader->Uniform("lightColors[" + std::to_string(i) + "]")
  //     ->SetFloat3(lightColors[i]);
  //
  //   model = glm::mat4(1.0f);
  //   model = glm::translate(model, newPos);
  //   model = glm::scale(model, glm::vec3(0.5f));
  //   PbrShader->Uniform("model")->SetMat4(model);
  //   PbrShader->Uniform("normalMatrix")
  //     ->SetMat3(glm::transpose(glm::inverse(glm::mat3(model))));
  //   Sphere->Draw(GL_TRIANGLE_STRIP, SphereDrawCount);
  // }

  Env->DrawSkybox(projection, view);
}
