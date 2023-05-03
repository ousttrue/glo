#include "renderer.h"
#include "env.h"
#include "mesh.h"
#include "pbrmaterial.h"
#include <learnopengl/filesystem.h>
#include <stdexcept>

Renderer::Renderer()
{
  {
    auto pbrShader = grapho::gl3::ShaderProgram::CreateFromPath("2.2.2.pbr.vs",
                                                                "2.2.2.pbr.fs");
    if (!pbrShader) {
      throw std::runtime_error(pbrShader.error());
    }
    PbrShader = *pbrShader;
  }

  {
    auto sphere = Mesh::Sphere();

    auto vbo = grapho::gl3::Vbo::Create(sphere->Vertices);
    std::shared_ptr<grapho::gl3::Vbo> slots[]{
      vbo,
    };
    auto ibo = grapho::gl3::Ibo::Create(sphere->Indices);

    Sphere = grapho::gl3::Vao::Create(sphere->Layouts, slots, ibo);
    SphereDrawCount = sphere->Indices.size();
  }

  // configure global opengl state
  // -----------------------------
  glEnable(GL_DEPTH_TEST);
  // set depth function to less than AND equal for skybox depth trick.
  glDepthFunc(GL_LEQUAL);
  // enable seamless cubemap sampling for lower mip levels in the pre-filter
  // map.
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

  // build and compile shaders
  // -------------------------

  PbrShader->Use();
  PbrShader->Uniform("irradianceMap")->SetInt(0);
  PbrShader->Uniform("prefilterMap")->SetInt(1);
  PbrShader->Uniform("brdfLUT")->SetInt(2);
  PbrShader->Uniform("albedoMap")->SetInt(3);
  PbrShader->Uniform("normalMap")->SetInt(4);
  PbrShader->Uniform("metallicMap")->SetInt(5);
  PbrShader->Uniform("roughnessMap")->SetInt(6);
  PbrShader->Uniform("aoMap")->SetInt(7);

  // load PBR material textures
  // --------------------------
  // rusted iron
  Iron = PbrMaterial::Create(
    FileSystem::getPath("resources/textures/pbr/rusted_iron/albedo.png"),
    FileSystem::getPath("resources/textures/pbr/rusted_iron/normal.png"),
    FileSystem::getPath("resources/textures/pbr/rusted_iron/metallic.png"),
    FileSystem::getPath("resources/textures/pbr/rusted_iron/roughness.png"),
    FileSystem::getPath("resources/textures/pbr/rusted_iron/ao.png"));

  // gold
  Gold = PbrMaterial::Create(
    FileSystem::getPath("resources/textures/pbr/gold/albedo.png"),
    FileSystem::getPath("resources/textures/pbr/gold/normal.png"),
    FileSystem::getPath("resources/textures/pbr/gold/metallic.png"),
    FileSystem::getPath("resources/textures/pbr/gold/roughness.png"),
    FileSystem::getPath("resources/textures/pbr/gold/ao.png"));

  // grass
  Grass = PbrMaterial::Create(
    FileSystem::getPath("resources/textures/pbr/grass/albedo.png"),
    FileSystem::getPath("resources/textures/pbr/grass/normal.png"),
    FileSystem::getPath("resources/textures/pbr/grass/metallic.png"),
    FileSystem::getPath("resources/textures/pbr/grass/roughness.png"),
    FileSystem::getPath("resources/textures/pbr/grass/ao.png"));

  // plastic
  Plastic = PbrMaterial::Create(
    FileSystem::getPath("resources/textures/pbr/plastic/albedo.png"),
    FileSystem::getPath("resources/textures/pbr/plastic/normal.png"),
    FileSystem::getPath("resources/textures/pbr/plastic/metallic.png"),
    FileSystem::getPath("resources/textures/pbr/plastic/roughness.png"),
    FileSystem::getPath("resources/textures/pbr/plastic/ao.png"));

  // wall
  Wall = PbrMaterial::Create(
    FileSystem::getPath("resources/textures/pbr/wall/albedo.png"),
    FileSystem::getPath("resources/textures/pbr/wall/normal.png"),
    FileSystem::getPath("resources/textures/pbr/wall/metallic.png"),
    FileSystem::getPath("resources/textures/pbr/wall/roughness.png"),
    FileSystem::getPath("resources/textures/pbr/wall/ao.png"));

  Env = std::make_shared<Environment>();
}

Renderer::~Renderer() {}

void
Renderer::Render(float currentFrame,
                 int scrWidth,
                 int scrHeight,
                 const DirectX::XMFLOAT4X4& projection,
                 const DirectX::XMFLOAT4X4& view,
                 const DirectX::XMFLOAT3& cameraPos)
{
  glViewport(0, 0, scrWidth, scrHeight);

  // per-frame time logic
  // --------------------
  deltaTime = currentFrame - lastFrame;
  lastFrame = currentFrame;

  // render
  // ------
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // render scene, supplying the convoluted irradiance map to the final
  // shader.
  // ------------------------------------------------------------------------------------------
  PbrShader->Use();
  glm::mat4 model = glm::mat4(1.0f);
  // auto view = cameraViewMatrix();
  PbrShader->Uniform("view")->SetMat4(view);
  // auto cameraPos = cameraPosition();
  PbrShader->Uniform("camPos")->SetFloat3(cameraPos);

  // initialize static shader uniforms before rendering
  // --------------------------------------------------
  PbrShader->Uniform("projection")->SetMat4(projection);

  // bind pre-computed IBL data
  Env->Bind();

  // rusted iron
  Iron->Bind();
  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(-5.0, 0.0, 2.0));
  PbrShader->Uniform("model")->SetMat4(model);
  PbrShader->Uniform("normalMatrix")
    ->SetMat3(glm::transpose(glm::inverse(glm::mat3(model))));
  Sphere->Draw(GL_TRIANGLE_STRIP, SphereDrawCount);

  // gold
  Gold->Bind();
  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(-3.0, 0.0, 2.0));
  PbrShader->Uniform("model")->SetMat4(model);
  PbrShader->Uniform("normalMatrix")
    ->SetMat3(glm::transpose(glm::inverse(glm::mat3(model))));
  Sphere->Draw(GL_TRIANGLE_STRIP, SphereDrawCount);

  // grass
  Grass->Bind();
  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(-1.0, 0.0, 2.0));
  PbrShader->Uniform("model")->SetMat4(model);
  PbrShader->Uniform("normalMatrix")
    ->SetMat3(glm::transpose(glm::inverse(glm::mat3(model))));
  Sphere->Draw(GL_TRIANGLE_STRIP, SphereDrawCount);

  // plastic
  Plastic->Bind();
  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(1.0, 0.0, 2.0));
  PbrShader->Uniform("model")->SetMat4(model);
  PbrShader->Uniform("normalMatrix")
    ->SetMat3(glm::transpose(glm::inverse(glm::mat3(model))));
  Sphere->Draw(GL_TRIANGLE_STRIP, SphereDrawCount);

  // wall
  Wall->Bind();
  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(3.0, 0.0, 2.0));
  PbrShader->Uniform("model")->SetMat4(model);
  PbrShader->Uniform("normalMatrix")
    ->SetMat3(glm::transpose(glm::inverse(glm::mat3(model))));
  Sphere->Draw(GL_TRIANGLE_STRIP, SphereDrawCount);

  // render light source (simply re-render sphere at light positions)
  // this looks a bit off as we use the same shader, but it'll make their
  // positions obvious and keeps the codeprint small.
  for (unsigned int i = 0;
       i < sizeof(lightPositions) / sizeof(lightPositions[0]);
       ++i) {
    glm::vec3 newPos =
      lightPositions[i] + glm::vec3(sin(currentFrame * 5.0) * 5.0, 0.0, 0.0);
    newPos = lightPositions[i];
    PbrShader->Uniform("lightPositions[" + std::to_string(i) + "]")
      ->SetFloat3(newPos);
    PbrShader->Uniform("lightColors[" + std::to_string(i) + "]")
      ->SetFloat3(lightColors[i]);

    model = glm::mat4(1.0f);
    model = glm::translate(model, newPos);
    model = glm::scale(model, glm::vec3(0.5f));
    PbrShader->Uniform("model")->SetMat4(model);
    PbrShader->Uniform("normalMatrix")
      ->SetMat3(glm::transpose(glm::inverse(glm::mat3(model))));
    Sphere->Draw(GL_TRIANGLE_STRIP, SphereDrawCount);
  }

  Env->DrawSkybox(projection, view);
}
