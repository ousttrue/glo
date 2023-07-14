#include <DirectXMath.h>
#include <cmath>

namespace grapho {
namespace camera {

struct Ray
{
  DirectX::XMFLOAT3 Origin;
  DirectX::XMFLOAT3 Direction;

  bool IsValid() const
  {
    if (!std::isfinite(Direction.x)) {
      return false;
    }
    if (!std::isfinite(Direction.y)) {
      return false;
    }
    if (!std::isfinite(Direction.z)) {
      return false;
    }
    return true;
  }

  Ray Transform(const DirectX::XMMATRIX& m) const
  {
    Ray ray;
    DirectX::XMStoreFloat3(
      &ray.Origin,
      DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&Origin), m));
    DirectX::XMStoreFloat3(
      &ray.Direction,
      DirectX::XMVector3Normalize(DirectX::XMVector3TransformNormal(
        DirectX::XMLoadFloat3(&Direction), m)));
    return ray;
  }
};

} // namespace
} // namespace
