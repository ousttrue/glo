#pragma once
#include <grapho/image.h>
#include <stdint.h>
#include <string>

struct ImageLoader
{
  grapho::Image Image;
  int nrComponents = 0;
  ImageLoader(const ImageLoader&) = delete;
  ImageLoader& operator=(const ImageLoader&) = delete;
  ImageLoader() {}
  ~ImageLoader();

  bool Load(const std::string& path);
  bool LoadHdr(const std::string& path);
};
