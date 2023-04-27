#pragma once
#include <DirectXMath.h>
#include <array>
#include <cmath>

namespace grapho {

struct OrbitView
{
  int Width = 1;
  int Height = 1;
  float FovY = DirectX::XMConvertToRadians(30.0f);
  float NearZ = 0.01f;
  float FarZ = 1000.0f;

  float Yaw = {};
  float Pitch = {};
  float shift_[3] = { 0, -0.8f, -5 };

  OrbitView() {}

  void SetSize(int w, int h)
  {
    if (w == Width && h == Height) {
      return;
    }
    Width = w;
    Height = h;
  }

  void YawPitch(int dx, int dy)
  {
    Yaw += DirectX::XMConvertToRadians(static_cast<float>(dx));
    Pitch += DirectX::XMConvertToRadians(static_cast<float>(dy));
  }

  void Shift(int dx, int dy)
  {
    auto factor = std::tan(FovY * 0.5f) * 2.0f * shift_[2] / Height;
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
    float aspectRatio = (float)Width / (float)Height;
    DirectX::XMStoreFloat4x4(
      (DirectX::XMFLOAT4X4*)projection,
      DirectX::XMMatrixPerspectiveFovRH(FovY, aspectRatio, NearZ, FarZ));

    auto yaw = DirectX::XMMatrixRotationY(Yaw);
    auto pitch = DirectX::XMMatrixRotationX(Pitch);
    auto shift = DirectX::XMMatrixTranslation(shift_[0], shift_[1], shift_[2]);
    DirectX::XMStoreFloat4x4((DirectX::XMFLOAT4X4*)view, yaw * pitch * shift);
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
    shift_[1] = -height * 0.5f;
    shift_[2] = -distance * 1.2f;
    auto r =
      DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(
        DirectX::XMLoadFloat3(&min), DirectX::XMLoadFloat3(&max))));
    NearZ = r * 0.01f;
    FarZ = r * 100.0f;
  }
};
}
