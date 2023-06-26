#pragma once
#include <array>

namespace grapho {
namespace camera {
struct Viewport
{
  int Width = 0;
  int Height = 0;
  std::array<float, 4> Color = { 1, 0, 1, 0 };
  float Depth = 1.0f;

  float AspectRatio() const { return (float)Width / (float)Height; }
};

} // namespace
} // namespace
