#pragma once
#include "dxmath_stub.h"

namespace grapho {

struct EuclideanTransform
{
  DirectX::XMFLOAT4 Rotation = { 0, 0, 0, 1 };
  DirectX::XMFLOAT3 Translation = { 0, 0, 0 };

  static EuclideanTransform Store(DirectX::XMVECTOR r, DirectX::XMVECTOR t);

  bool HasRotation() const
  {
    if (Rotation.x == 0 && Rotation.y == 0 && Rotation.z == 0 &&
        (Rotation.w == 1 || Rotation.w == -1)) {
      return false;
    }
    return true;
  }

  DirectX::XMMATRIX ScalingTranslationMatrix(float scaling) const;

  DirectX::XMMATRIX Matrix() const;

  EuclideanTransform& SetMatrix(DirectX::XMMATRIX m);

  DirectX::XMMATRIX InversedMatrix() const;

  EuclideanTransform Invrsed() const;

  EuclideanTransform Rotate(DirectX::XMVECTOR r);
};

} // namespace
