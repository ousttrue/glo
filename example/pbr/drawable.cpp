#include <GL/glew.h>

#include "drawable.h"
#include "image.h"
#include <grapho/gl3/pbr.h>
#include <grapho/gl3/vao.h>
#include <grapho/mesh.h>

Drawable::Drawable()
{
  auto sphere = grapho::mesh::Sphere();
  Mesh = grapho::gl3::Vao::Create(sphere);
  MeshDrawCount = sphere->DrawCount();
  MeshDrawMode = *grapho::gl3::GLMode(sphere->Mode);
}

void
Drawable::Draw(const DirectX::XMFLOAT4X4& projection,
               const DirectX::XMFLOAT4X4& view,
               const DirectX::XMFLOAT3& cameraPos,
               uint32_t UBO_LIGHTS_BINDING)
{
  DirectX::XMFLOAT4X4 model;
  DirectX::XMStoreFloat4x4(
    &model, DirectX::XMMatrixTranslation(Position.x, Position.y, Position.z));
  Material->Activate(projection, view, model, cameraPos, UBO_LIGHTS_BINDING);
  Mesh->Draw(MeshDrawMode, MeshDrawCount);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
static std::shared_ptr<grapho::gl3::Texture>
loadTexture(const std::filesystem::path& path)
{
  Image image;
  if (!image.Load(path)) {
    return {};
  }

  auto texture = grapho::gl3::Texture::Create(
    image.Width, image.Height, image.Format, image.Data);
  texture->SamplingLinear(true);
  texture->WrapRepeat();
  return texture;
}

void
Scene::Load(const std::filesystem::path& baseDir)
{
  auto iron = std::make_shared<Drawable>();
  iron->Material = grapho::gl3::PbrMaterial::Create(
    loadTexture(baseDir / "resources/textures/pbr/rusted_iron/albedo.png"),
    loadTexture(baseDir / "resources/textures/pbr/rusted_iron/normal.png"),
    loadTexture(baseDir / "resources/textures/pbr/rusted_iron/metallic.png"),
    loadTexture(baseDir / "resources/textures/pbr/rusted_iron/roughness.png"),
    loadTexture(baseDir / "resources/textures/pbr/rusted_iron/ao.png"));
  iron->Position = { -5.0, 0.0, 2.0 };
  Drawables.push_back(iron);

  auto gold = std::make_shared<Drawable>();
  gold->Material = grapho::gl3::PbrMaterial::Create(
    loadTexture(baseDir / "resources/textures/pbr/gold/albedo.png"),
    loadTexture(baseDir / "resources/textures/pbr/gold/normal.png"),
    loadTexture(baseDir / "resources/textures/pbr/gold/metallic.png"),
    loadTexture(baseDir / "resources/textures/pbr/gold/roughness.png"),
    loadTexture(baseDir / "resources/textures/pbr/gold/ao.png"));
  gold->Position = { -3.0, 0.0, 2.0 };
  Drawables.push_back(gold);

  auto grass = std::make_shared<Drawable>();
  grass->Material = grapho::gl3::PbrMaterial::Create(
    loadTexture(baseDir / "resources/textures/pbr/grass/albedo.png"),
    loadTexture(baseDir / "resources/textures/pbr/grass/normal.png"),
    loadTexture(baseDir / "resources/textures/pbr/grass/metallic.png"),
    loadTexture(baseDir / "resources/textures/pbr/grass/roughness.png"),
    loadTexture(baseDir / "resources/textures/pbr/grass/ao.png"));
  grass->Position = { -1.0, 0.0, 2.0 };
  Drawables.push_back(grass);

  auto plastic = std::make_shared<Drawable>();
  plastic->Material = grapho::gl3::PbrMaterial::Create(
    loadTexture(baseDir / "resources/textures/pbr/plastic/albedo.png"),
    loadTexture(baseDir / "resources/textures/pbr/plastic/normal.png"),
    loadTexture(baseDir / "resources/textures/pbr/plastic/metallic.png"),
    loadTexture(baseDir / "resources/textures/pbr/plastic/roughness.png"),
    loadTexture(baseDir / "resources/textures/pbr/plastic/ao.png"));
  plastic->Position = { 1.0, 0.0, 2.0 };
  Drawables.push_back(plastic);

  auto wall = std::make_shared<Drawable>();
  wall->Material = grapho::gl3::PbrMaterial::Create(
    loadTexture(baseDir / "resources/textures/pbr/wall/albedo.png"),
    loadTexture(baseDir / "resources/textures/pbr/wall/normal.png"),
    loadTexture(baseDir / "resources/textures/pbr/wall/metallic.png"),
    loadTexture(baseDir / "resources/textures/pbr/wall/roughness.png"),
    loadTexture(baseDir / "resources/textures/pbr/wall/ao.png"));
  wall->Position = { 3.0, 0.0, 2.0 };
  Drawables.push_back(wall);
}
