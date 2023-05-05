#include <GL/glew.h>

#include "image.h"
#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include <stb_image.h>

Image::~Image()
{
  stbi_image_free(Data);
}

bool
Image::Load(const std::filesystem::path& path)
{
  Data = stbi_load(path.string().c_str(), &Width, &Height, &nrComponents, 0);
  if (!Data) {
    std::cout << "Texture failed to load at path: " << path << std::endl;
    return false;
  }

  switch (nrComponents) {
    case 1:
      Format = grapho::PixelFormat::u8_R;
      break;
    case 3:
      Format = grapho::PixelFormat::u8_RGB;
      break;
    case 4:
      Format = grapho::PixelFormat::u8_RGBA;
      break;

    default:
      return false;
  }

  return true;
}

bool
Image::LoadHdr(const std::filesystem::path& path)
{
  stbi_set_flip_vertically_on_load(true);
  // int width, height, nrComponents;
  Data = (uint8_t*)stbi_loadf(
    path.string().c_str(), &Width, &Height, &nrComponents, 0);
  if (!Data) {
    std::cout << "Texture failed to load at path: " << path << std::endl;
    return false;
  }

  assert(nrComponents == 3);
  Format = grapho::PixelFormat::f16_RGB;

  return true;
}
