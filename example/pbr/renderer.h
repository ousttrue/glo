#pragma once
#include <DirectXMath.h>
#include <memory>
#include <vector>

struct Environment;
struct Drawable;
struct Light;

class Renderer
{
  // float deltaTime = 0.0f;
  // float lastFrame = 0.0f;
  std::shared_ptr<Environment> Env;
  std::vector<std::shared_ptr<Drawable>> Drawables;
  std::vector<std::shared_ptr<Light>> Lights;

public:
  Renderer();
  ~Renderer();
  void Render(int w,
              int h,
              const DirectX::XMFLOAT4X4& projection,
              const DirectX::XMFLOAT4X4& view,
              const DirectX::XMFLOAT3& cameraPos);
};
