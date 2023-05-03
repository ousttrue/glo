#pragma once
#include <GL/glew.h>
#include <learnopengl/shader.h>

class Renderer
{
  Shader pbrShader;
  Shader backgroundShader;
  unsigned int envCubemap;
  unsigned int irradianceMap;
  unsigned int prefilterMap;
  unsigned int brdfLUTTexture;

  unsigned int ironAlbedoMap;
  unsigned int ironNormalMap;
  unsigned int ironMetallicMap;
  unsigned int ironRoughnessMap;
  unsigned int ironAOMap;

  unsigned int goldAlbedoMap;
  unsigned int goldNormalMap;
  unsigned int goldMetallicMap;
  unsigned int goldRoughnessMap;
  unsigned int goldAOMap;

  unsigned int grassAlbedoMap;
  unsigned int grassNormalMap;
  unsigned int grassMetallicMap;
  unsigned int grassRoughnessMap;
  unsigned int grassAOMap;

  unsigned int plasticAlbedoMap;
  unsigned int plasticNormalMap;
  unsigned int plasticMetallicMap;
  unsigned int plasticRoughnessMap;
  unsigned int plasticAOMap;

  unsigned int wallAlbedoMap;
  unsigned int wallNormalMap;
  unsigned int wallMetallicMap;
  unsigned int wallRoughnessMap;
  unsigned int wallAOMap;

  // lights
  // ------
  glm::vec3 lightPositions[4] = {
    glm::vec3(-10.0f, 10.0f, 10.0f),
    glm::vec3(10.0f, 10.0f, 10.0f),
    glm::vec3(-10.0f, -10.0f, 10.0f),
    glm::vec3(10.0f, -10.0f, 10.0f),
  };

  glm::vec3 lightColors[4] = { glm::vec3(300.0f, 300.0f, 300.0f),
                               glm::vec3(300.0f, 300.0f, 300.0f),
                               glm::vec3(300.0f, 300.0f, 300.0f),
                               glm::vec3(300.0f, 300.0f, 300.0f) };

public:
  Renderer();
  ~Renderer();
  void Render(int w, int h);
};
