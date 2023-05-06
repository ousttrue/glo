#pragma once
#include "pixelformat.h"

namespace grapho {

enum class ColorSpace
{
  sRGB,
  Linear,
};

struct Image
{
  int Width;
  int Height;
  PixelFormat Format;
  ColorSpace ColorSpace = ColorSpace::sRGB;
  const uint8_t* Pixels = nullptr;
};

}
