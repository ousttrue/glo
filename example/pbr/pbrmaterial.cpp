#include <GL/glew.h>

#include "pbrmaterial.h"
#include "image.h"

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
