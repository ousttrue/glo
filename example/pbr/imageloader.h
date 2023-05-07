#pragma once
#include <filesystem>
#include <grapho/image.h>
#include <stdint.h>

struct ImageLoader
{
  grapho::Image Image;
  int nrComponents = 0;
  ImageLoader(const ImageLoader&) = delete;
  ImageLoader& operator=(const ImageLoader&) = delete;
  ImageLoader() {}
  ~ImageLoader();

  bool Load(const std::filesystem::path& path);
  bool LoadHdr(const std::filesystem::path& path);
};
