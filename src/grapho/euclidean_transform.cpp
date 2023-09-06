#include <DirectXMath.h>

#include "euclidean_transform.h"

namespace grapho {

EuclideanTransform
EuclideanTransform::Store(DirectX::XMVECTOR r, DirectX::XMVECTOR t)
{
  EuclideanTransform transform;
  DirectX::XMStoreFloat4(&transform.Rotation, r);
  DirectX::XMStoreFloat3(&transform.Translation, t);
  return transform;
}

DirectX::XMMATRIX
EuclideanTransform::ScalingTranslationMatrix(float scaling) const
{
  auto r =
    DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&Rotation));
  auto t = DirectX::XMMatrixTranslation(
    Translation.x * scaling, Translation.y * scaling, Translation.z * scaling);
  return r * t;
}

DirectX::XMMATRIX
EuclideanTransform::Matrix() const
{
  auto r =
    DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&Rotation));
  auto t =
    DirectX::XMMatrixTranslation(Translation.x, Translation.y, Translation.z);
  return r * t;
}

EuclideanTransform&
EuclideanTransform::SetMatrix(DirectX::XMMATRIX m)
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

DirectX::XMMATRIX
EuclideanTransform::InversedMatrix() const
{
  auto r = DirectX::XMMatrixRotationQuaternion(
    DirectX::XMQuaternionInverse(DirectX::XMLoadFloat4(&Rotation)));
  auto t = DirectX::XMMatrixTranslation(
    -Translation.x, -Translation.y, -Translation.z);
  return t * r;
}

EuclideanTransform
EuclideanTransform::Invrsed() const
{
  auto r = DirectX::XMQuaternionInverse(DirectX::XMLoadFloat4(&Rotation));
  auto t = DirectX::XMVector3Rotate(
    DirectX::XMVectorSet(-Translation.x, -Translation.y, -Translation.z, 1), r);
  return Store(r, t);
}

EuclideanTransform
EuclideanTransform::Rotate(DirectX::XMVECTOR r)
{
  return EuclideanTransform::Store(
    DirectX::XMQuaternionMultiply(DirectX::XMLoadFloat4(&Rotation), r),
    DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&Translation),
                                DirectX::XMMatrixRotationQuaternion(r)));
}

} // namespace
