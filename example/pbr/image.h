#pragma once
#include <filesystem>
#include <stdint.h>

struct Image
{
  int Width = 0;
  int Height = 0;
  int nrComponents = 0;
  uint8_t* Data = nullptr;
  uint32_t Format = 0;
  Image(const Image&) = delete;
  Image& operator=(const Image&) = delete;
  Image() {}
  ~Image();

  bool Load(const std::filesystem::path& path);
  bool LoadHdr(const std::filesystem::path& path);
};
