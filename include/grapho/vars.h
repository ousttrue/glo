#pragma once
#include <DirectXMath.h>

namespace grapho {

struct WorldVars
{
  DirectX::XMFLOAT4X4 projection;
  DirectX::XMFLOAT4X4 view;
  DirectX::XMFLOAT4 lightPositions[4];
  DirectX::XMFLOAT4 lightColors[4];
  DirectX::XMFLOAT4 camPos;

  DirectX::XMFLOAT4X4 viewprojection() const
  {
    DirectX::XMFLOAT4X4 m;
    DirectX::XMStoreFloat4x4(&m,
                             DirectX::XMLoadFloat4x4(&view) *
                               DirectX::XMLoadFloat4x4(&projection));
    return m;
  }
};

struct LocalVars
{
  DirectX::XMFLOAT4X4 model;
  DirectX::XMFLOAT4 color;
  DirectX::XMFLOAT4 cutoff;
  DirectX::XMFLOAT4X4 normalMatrix;
  DirectX::XMFLOAT3 emissiveColor;
  DirectX::XMFLOAT3X3 normalMatrix3() const
  {
    DirectX::XMFLOAT3X3 m;
    DirectX::XMStoreFloat3x3(&m, DirectX::XMLoadFloat4x4(&normalMatrix));
    return m;
  }
  DirectX::XMFLOAT3X3 uvTransform() const
  {
    DirectX::XMFLOAT3X3 m;
    DirectX::XMStoreFloat3x3(&m, DirectX::XMMatrixIdentity());
    return m;
  }

  void CalcNormalMatrix()
  {
    auto m = DirectX::XMLoadFloat4x4(&model);
    DirectX::XMVECTOR det;
    auto ti = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&det, m));
    // DirectX::XMFLOAT3X3 mat3;
    DirectX::XMStoreFloat4x4(&normalMatrix, ti);
    // normalMatrix._41 = 0;
    // normalMatrix._42 = 0;
    // normalMatrix._43 = 0;
    // return mat3;
  }
};

}
