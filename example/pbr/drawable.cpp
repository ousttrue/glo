#include <GL/glew.h>

#include "drawable.h"
#include "imageloader.h"
#include <grapho/gl3/pbr.h>
#include <grapho/gl3/vao.h>
#include <grapho/mesh.h>

Drawable::Drawable()
{
  auto sphere = grapho::mesh::Sphere();
  Mesh = grapho::gl3::Vao::Create(sphere);
  MeshDrawCount = sphere->DrawCount();
  MeshDrawMode = *grapho::gl3::GLMode(sphere->Mode);
  Ubo = grapho::gl3::Ubo::Create(sizeof(Vars), nullptr);
}

void
Drawable::Draw(uint32_t world_ubo_binding)
{
  DirectX::XMStoreFloat4x4(
    &Vars.model,
    DirectX::XMMatrixTranslation(Position.x, Position.y, Position.z));
  Vars.CalcNormalMatrix();
  Ubo->Upload(Vars);
  Ubo->Bind();
  Ubo->SetBindingPoint(1);
  Material->Activate(world_ubo_binding, 1);
  Mesh->Draw(MeshDrawMode, MeshDrawCount);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
static std::shared_ptr<grapho::gl3::Texture>
loadTexture(const std::filesystem::path& path, grapho::ColorSpace colorspace)
{
  ImageLoader loader;
  if (!loader.Load(path)) {
    return {};
  }
  loader.Image.ColorSpace = colorspace;

  auto texture = grapho::gl3::Texture::Create(loader.Image);
  texture->SamplingLinear(true);
  texture->WrapRepeat();
  return texture;
}

void
Scene::Load(const std::filesystem::path& baseDir)
{
  if (auto material = grapho::gl3::CreatePbrMaterial(
        loadTexture(baseDir / "resources/textures/pbr/rusted_iron/albedo.png",
                    grapho::ColorSpace::Linear),
        loadTexture(baseDir / "resources/textures/pbr/rusted_iron/normal.png",
                    grapho::ColorSpace::Linear),
        loadTexture(baseDir / "resources/textures/pbr/rusted_iron/metallic.png",
                    grapho::ColorSpace::Linear),
        loadTexture(baseDir /
                      "resources/textures/pbr/rusted_iron/roughness.png",
                    grapho::ColorSpace::Linear),
        loadTexture(baseDir / "resources/textures/pbr/rusted_iron/ao.png",
                    grapho::ColorSpace::Linear))) {
    auto iron = std::make_shared<Drawable>();
    iron->Material = *material;
    iron->Position = { -5.0, 0.0, 2.0 };
    Drawables.push_back(iron);
  }

  if (auto material = grapho::gl3::CreatePbrMaterial(
        loadTexture(baseDir / "resources/textures/pbr/gold/albedo.png",
                    grapho::ColorSpace::Linear),
        loadTexture(baseDir / "resources/textures/pbr/gold/normal.png",
                    grapho::ColorSpace::Linear),
        loadTexture(baseDir / "resources/textures/pbr/gold/metallic.png",
                    grapho::ColorSpace::Linear),
        loadTexture(baseDir / "resources/textures/pbr/gold/roughness.png",
                    grapho::ColorSpace::Linear),
        loadTexture(baseDir / "resources/textures/pbr/gold/ao.png",
                    grapho::ColorSpace::Linear))) {
    auto gold = std::make_shared<Drawable>();
    gold->Material = *material;
    gold->Position = { -3.0, 0.0, 2.0 };
    Drawables.push_back(gold);
  }

  if (auto material = grapho::gl3::CreatePbrMaterial(
        loadTexture(baseDir / "resources/textures/pbr/grass/albedo.png",
                    grapho::ColorSpace::Linear),
        loadTexture(baseDir / "resources/textures/pbr/grass/normal.png",
                    grapho::ColorSpace::Linear),
        loadTexture(baseDir / "resources/textures/pbr/grass/metallic.png",
                    grapho::ColorSpace::Linear),
        loadTexture(baseDir / "resources/textures/pbr/grass/roughness.png",
                    grapho::ColorSpace::Linear),
        loadTexture(baseDir / "resources/textures/pbr/grass/ao.png",
                    grapho::ColorSpace::Linear))) {
    auto grass = std::make_shared<Drawable>();
    grass->Material = *material;
    grass->Position = { -1.0, 0.0, 2.0 };
    Drawables.push_back(grass);
  }

  if (auto material = grapho::gl3::CreatePbrMaterial(
        loadTexture(baseDir / "resources/textures/pbr/plastic/albedo.png",
                    grapho::ColorSpace::Linear),
        loadTexture(baseDir / "resources/textures/pbr/plastic/normal.png",
                    grapho::ColorSpace::Linear),
        loadTexture(baseDir / "resources/textures/pbr/plastic/metallic.png",
                    grapho::ColorSpace::Linear),
        loadTexture(baseDir / "resources/textures/pbr/plastic/roughness.png",
                    grapho::ColorSpace::Linear),
        loadTexture(baseDir / "resources/textures/pbr/plastic/ao.png",
                    grapho::ColorSpace::Linear))) {
    auto plastic = std::make_shared<Drawable>();
    plastic->Material = *material;
    plastic->Position = { 1.0, 0.0, 2.0 };
    Drawables.push_back(plastic);
  }

  if (auto material = grapho::gl3::CreatePbrMaterial(
        loadTexture(baseDir / "resources/textures/pbr/wall/albedo.png",
                    grapho::ColorSpace::Linear),
        loadTexture(baseDir / "resources/textures/pbr/wall/normal.png",
                    grapho::ColorSpace::Linear),
        loadTexture(baseDir / "resources/textures/pbr/wall/metallic.png",
                    grapho::ColorSpace::Linear),
        loadTexture(baseDir / "resources/textures/pbr/wall/roughness.png",
                    grapho::ColorSpace::Linear),
        loadTexture(baseDir / "resources/textures/pbr/wall/ao.png",
                    grapho::ColorSpace::Linear))) {
    auto wall = std::make_shared<Drawable>();
    wall->Material = *material;
    wall->Position = { 3.0, 0.0, 2.0 };
    Drawables.push_back(wall);
  }
}
