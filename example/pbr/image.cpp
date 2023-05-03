#include <GL/glew.h>

#include "image.h"
#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include <stb_image.h>

Image::Image(std::string_view path)
{
  std::string p(path.begin(), path.end());
  Data = stbi_load(p.c_str(), &Width, &Height, &nrComponents, 0);
  if (Data) {
    if (nrComponents == 1)
      Format = GL_RED;
    else if (nrComponents == 3)
      Format = GL_RGB;
    else if (nrComponents == 4)
      Format = GL_RGBA;

  } else {
    std::cout << "Texture failed to load at path: " << path << std::endl;
  }
}

Image::~Image()
{
  stbi_image_free(Data);
}
