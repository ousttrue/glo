#pragma once
#include <memory>
#include <string_view>

struct PbrMaterial
{
  unsigned int AlbedoMap;
  unsigned int NormalMap;
  unsigned int MetallicMap;
  unsigned int RoughnessMap;
  unsigned int AOMap;

  PbrMaterial() {}
  ~PbrMaterial() {}
  PbrMaterial(const PbrMaterial&) = delete;
  PbrMaterial& operator=(const PbrMaterial&) = delete;

  static std::shared_ptr<PbrMaterial> Create(std::string_view texture_path,
                                             std::string_view normal,
                                             std::string_view metallic,
                                             std::string_view roughness,
                                             std::string_view ao);
  void Bind();
};
