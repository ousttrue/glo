#include <DirectXMath.h>
#include <GL/glew.h>

#include "drawable.h"
#include "imageloader.h"
#include <grapho/gl3/pbr.h>
#include <grapho/gl3/vao.h>
#include <grapho/gl3/ubo.h>
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
    (DirectX::XMFLOAT4X4*)&Vars.model,
    DirectX::XMMatrixTranslation(Position.x, Position.y, Position.z));
  Vars.CalcNormalMatrix();
  Ubo->Upload(Vars);
  Ubo->Bind();
  Ubo->SetBindingPoint(1);
  Shader->Use();
  Shader->UboBind(0, world_ubo_binding);
  Shader->UboBind(1, 1);
  for (uint32_t i = 0; i < Textures.size(); ++i) {
    Textures[i]->Activate(i + 3);
  }
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

std::shared_ptr<Drawable>
Drawable::Load(const std::filesystem::path& baseDir,
               const grapho::XMFLOAT3& position)
{
  auto drawable = std::make_shared<Drawable>();
  drawable->Shader = grapho::gl3::CreatePbrShader();
  if (!drawable->Shader) {
    return {};
  }
  drawable->Position = position;
  drawable->Textures = {
    loadTexture(baseDir / "albedo.png", grapho::ColorSpace::Linear),
    loadTexture(baseDir / "normal.png", grapho::ColorSpace::Linear),
    loadTexture(baseDir / "metallic.png", grapho::ColorSpace::Linear),
    loadTexture(baseDir / "roughness.png", grapho::ColorSpace::Linear),
    loadTexture(baseDir / "ao.png", grapho::ColorSpace::Linear),
  };
  return drawable;
}
