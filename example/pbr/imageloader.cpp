#include <GL/glew.h>

#include "imageloader.h"
#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include <stb_image.h>

ImageLoader::~ImageLoader()
{
  stbi_image_free((void*)Image.Pixels);
}

bool
ImageLoader::Load(const std::string& path)
{
  Image.Pixels =
    stbi_load(path.c_str(), &Image.Width, &Image.Height, &nrComponents, 0);
  if (!Image.Pixels) {
    std::cout << "Texture failed to load at path: " << path << std::endl;
    return false;
  }

  Image.ColorSpace = grapho::ColorSpace::sRGB;
  switch (nrComponents) {
    case 1:
      Image.Format = grapho::PixelFormat::u8_R;
      break;
    case 3:
      Image.Format = grapho::PixelFormat::u8_RGB;
      break;
    case 4:
      Image.Format = grapho::PixelFormat::u8_RGBA;
      break;

    default:
      return false;
  }

  return true;
}

bool
ImageLoader::LoadHdr(const std::string& path)
{
  stbi_set_flip_vertically_on_load(true);
  // int width, height, nrComponents;
  Image.Pixels = (uint8_t*)stbi_loadf(
    path.c_str(), &Image.Width, &Image.Height, &nrComponents, 0);
  if (!Image.Pixels) {
    std::cout << "Texture failed to load at path: " << path << std::endl;
    return false;
  }

  assert(nrComponents == 3);
  Image.ColorSpace = grapho::ColorSpace::Linear;
  Image.Format = grapho::PixelFormat::f16_RGB;

  return true;
}
