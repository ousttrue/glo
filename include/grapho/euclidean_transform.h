#pragma once
#include <DirectXMath.h>

namespace grapho {

struct EuclideanTransform
{
  DirectX::XMFLOAT4 Rotation = { 0, 0, 0, 1 };
  DirectX::XMFLOAT3 Translation = { 0, 0, 0 };

  static EuclideanTransform Store(DirectX::XMVECTOR r, DirectX::XMVECTOR t)
  {
    EuclideanTransform transform;
    DirectX::XMStoreFloat4(&transform.Rotation, r);
    DirectX::XMStoreFloat3(&transform.Translation, t);
    return transform;
  }

  bool HasRotation() const
  {
    if (Rotation.x == 0 && Rotation.y == 0 && Rotation.z == 0 &&
        (Rotation.w == 1 || Rotation.w == -1)) {
      return false;
    }
    return true;
  }

  DirectX::XMMATRIX ScalingTranslationMatrix(float scaling) const
  {
    auto r =
      DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&Rotation));
    auto t = DirectX::XMMatrixTranslation(Translation.x * scaling,
                                          Translation.y * scaling,
                                          Translation.z * scaling);
    return r * t;
  }

  DirectX::XMMATRIX Matrix() const
  {
    auto r =
      DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&Rotation));
    auto t =
      DirectX::XMMatrixTranslation(Translation.x, Translation.y, Translation.z);
    return r * t;
  }

  EuclideanTransform& SetMatrix(DirectX::XMMATRIX m)
  {
    DirectX::XMVECTOR s;
    DirectX::XMVECTOR r;
    DirectX::XMVECTOR t;
    if (!DirectX::XMMatrixDecompose(&s, &r, &t, m)) {
      assert(false);
    }
    // DirectX::XMStoreFloat3((DirectX::XMFLOAT3*)&InitialScale, s);
    DirectX::XMStoreFloat4(&Rotation, r);
    DirectX::XMStoreFloat3(&Translation, t);
    return *this;
  }

  DirectX::XMMATRIX InversedMatrix() const
  {
    auto r = DirectX::XMMatrixRotationQuaternion(
      DirectX::XMQuaternionInverse(DirectX::XMLoadFloat4(&Rotation)));
    auto t = DirectX::XMMatrixTranslation(
      -Translation.x, -Translation.y, -Translation.z);
    return t * r;
  }

  EuclideanTransform Invrsed() const
  {
    auto r = DirectX::XMQuaternionInverse(DirectX::XMLoadFloat4(&Rotation));
    auto t = DirectX::XMVector3Rotate(
      DirectX::XMVectorSet(-Translation.x, -Translation.y, -Translation.z, 1),
      r);
    return Store(r, t);
  }

  EuclideanTransform Rotate(DirectX::XMVECTOR r)
  {
    return EuclideanTransform::Store(
      DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4(&Rotation), r),
      DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&Translation),
                                  DirectX::XMMatrixRotationQuaternion(r)));
  }
};

} // namespace
