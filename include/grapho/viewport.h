#pragma once

namespace grapho {
struct Viewport
{
  int Width = 0;
  int Height = 0;
  float Color[4] = { 1, 0, 1, 0 };
  float Depth = 1.0f;

  float AspectRatio() const { return (float)Width / (float)Height; }
};
}
