#pragma once
#include <stdint.h>
#include <string_view>

struct Image
{
  int Width = 0;
  int Height = 0;
  int nrComponents = 0;
  uint8_t* Data = nullptr;
  uint32_t Format = 0;
  Image(const Image&) = delete;
  Image& operator=(const Image&) = delete;
  Image(std::string_view path);
  ~Image();
};
