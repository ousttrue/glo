#include <GL/glew.h>

#include "ibl_specular_textured.h"
#include "pbrmaterial.h"

std::shared_ptr<PbrMaterial>
PbrMaterial::Create(std::string_view albedo,
                    std::string_view normal,
                    std::string_view metallic,
                    std::string_view roughness,
                    std::string_view ao)
{
  auto ptr = std::make_shared<PbrMaterial>();
  ptr->AlbedoMap = loadTexture(albedo);
  ptr->NormalMap = loadTexture(normal);
  ptr->MetallicMap = loadTexture(metallic);
  ptr->RoughnessMap = loadTexture(roughness);
  ptr->AOMap = loadTexture(ao);
  return ptr;
};

void
PbrMaterial::Bind()
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
}

