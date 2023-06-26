#pragma once
#include "viewport.h"
#include <DirectXMath.h>
#include <array>
#include <cmath>

namespace grapho {
namespace camera {

struct OrbitView
{
  Viewport Viewport;
  float FovY = DirectX::XMConvertToRadians(30.0f);
  float NearZ = 0.01f;
  float FarZ = 1000.0f;

  float Yaw = {};
  float Pitch = {};
  float shift_[3] = { 0, -0.8f, -5 };

  mutable DirectX::XMFLOAT3 Position = {};

  OrbitView() {}

  void SetSize(int w, int h)
  {
    if (w == Viewport.Width && h == Viewport.Height) {
      return;
    }
    Viewport.Width = w;
    Viewport.Height = h;
  }

  void YawPitch(int dx, int dy)
  {
    Yaw += DirectX::XMConvertToRadians(static_cast<float>(dx));
    Pitch += DirectX::XMConvertToRadians(static_cast<float>(dy));
  }

  void Shift(int dx, int dy)
  {
    auto factor = std::tan(FovY * 0.5f) * 2.0f * shift_[2] / Viewport.Height;
    shift_[0] -= dx * factor;
    shift_[1] += dy * factor;
  }

  void Dolly(int d)
  {
    if (d > 0) {
      shift_[2] *= 0.9f;
    } else if (d < 0) {
      shift_[2] *= 1.1f;
    }
  }

  void Update(const float projection[16], const float view[16]) const
  {
    auto aspectRatio = Viewport.AspectRatio();
    DirectX::XMStoreFloat4x4(
      (DirectX::XMFLOAT4X4*)projection,
      DirectX::XMMatrixPerspectiveFovRH(FovY, aspectRatio, NearZ, FarZ));

    auto yaw = DirectX::XMMatrixRotationY(Yaw);
    auto pitch = DirectX::XMMatrixRotationX(Pitch);
    auto shift = DirectX::XMMatrixTranslation(shift_[0], shift_[1], shift_[2]);
    auto v = yaw * pitch * shift;
    DirectX::XMStoreFloat4x4((DirectX::XMFLOAT4X4*)view, v);

    // Pos
    DirectX::XMVECTOR det;
    auto inv = DirectX::XMMatrixInverse(&det, v);
    DirectX::XMStoreFloat3(
      &Position, DirectX::XMVector3Transform(DirectX::XMVectorZero(), inv));
  }

  void Fit(const DirectX::XMFLOAT3& min, const DirectX::XMFLOAT3& max)
  {
    Yaw = {};
    Pitch = {};
    // shift_[3] = { 0, -0.8f, -5 };
    auto height = max.y - min.y;
    if (fabs(height) < 1e-4) {
      return;
    }
    auto distance = height * 0.5f / std::atan(FovY * 0.5f);
    shift_[0] = 0;
    shift_[1] = -(max.y + min.y) * 0.5f;
    shift_[2] = -distance * 1.2f;
    auto r =
      DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(
        DirectX::XMLoadFloat3(&min), DirectX::XMLoadFloat3(&max))));
    NearZ = r * 0.01f;
    FarZ = r * 100.0f;
  }
};

} // namespace
} // namespace