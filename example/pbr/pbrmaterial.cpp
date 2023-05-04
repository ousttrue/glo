#include <GL/glew.h>

#include "image.h"
#include "pbrmaterial.h"
#include "shaders/pbr_fs.h"
#include "shaders/pbr_vs.h"
#include <grapho/gl3/shader.h>
#include <grapho/gl3/ubo.h>

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int
loadTexture(std::string_view path)
{
  Image image(path);

  unsigned int textureID;
  glGenTextures(1, &textureID);

  glBindTexture(GL_TEXTURE_2D, textureID);
  glTexImage2D(GL_TEXTURE_2D,
               0,
               image.Format,
               image.Width,
               image.Height,
               0,
               image.Format,
               GL_UNSIGNED_BYTE,
               image.Data);
  glGenerateMipmap(GL_TEXTURE_2D);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(
    GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  return textureID;
}

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

PbrMaterial::PbrMaterial()
{
  auto shader = grapho::gl3::ShaderProgram::Create(PBR_VS, PBR_FS);
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
  LightsUbo = grapho::gl3::Ubo::Create(sizeof(Lights), nullptr);
}

std::shared_ptr<PbrMaterial>
PbrMaterial::Create(std::string_view albedo,
                    std::string_view normal,
                    std::string_view metallic,
                    std::string_view roughness,
                    std::string_view ao)
{
  auto ptr = std::make_shared<PbrMaterial>();
  {
  }
  ptr->AlbedoMap = loadTexture(albedo);
  ptr->NormalMap = loadTexture(normal);
  ptr->MetallicMap = loadTexture(metallic);
  ptr->RoughnessMap = loadTexture(roughness);
  ptr->AOMap = loadTexture(ao);
  return ptr;
};

void
PbrMaterial::Activate(const DirectX::XMFLOAT4X4& projection,
                      const DirectX::XMFLOAT4X4& view,
                      const DirectX::XMFLOAT3& position,
                      const DirectX::XMFLOAT3& cameraPos,
                      const Lights& lights)
{
  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, AlbedoMap);
  glActiveTexture(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_2D, NormalMap);
  glActiveTexture(GL_TEXTURE5);
  glBindTexture(GL_TEXTURE_2D, MetallicMap);
  glActiveTexture(GL_TEXTURE6);
  glBindTexture(GL_TEXTURE_2D, RoughnessMap);
  glActiveTexture(GL_TEXTURE7);
  glBindTexture(GL_TEXTURE_2D, AOMap);

  const uint32_t UBO_LIGHTS_BINDING = 0;

  LightsUbo->Upload(lights);
  LightsUbo->SetBindingPoint(UBO_LIGHTS_BINDING);

  Shader->Use();
  // auto view = cameraViewMatrix();
  Shader->Uniform("view")->SetMat4(view);
  // auto cameraPos = cameraPosition();
  Shader->Uniform("camPos")->SetFloat3(cameraPos);
  // initialize static shader uniforms before rendering
  Shader->Uniform("projection")->SetMat4(projection);

  auto uboBlock = Shader->UboBlockIndex("lights");
  Shader->UboBind(*uboBlock, UBO_LIGHTS_BINDING);

  DirectX::XMFLOAT4X4 model;
  DirectX::XMStoreFloat4x4(
    &model, DirectX::XMMatrixTranslation(position.x, position.y, position.z));
  Shader->Uniform("model")->SetMat4(model);
  Shader->Uniform("normalMatrix")->SetMat3(TransposeInv(model));
}
