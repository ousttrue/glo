#include <DirectXMath.h>

namespace grapho {
namespace camera {

struct Ray
{
  DirectX::XMFLOAT3 Origin;
  DirectX::XMFLOAT3 Direction;

  Ray Transform(const DirectX::XMMATRIX& m) const
  {
    Ray ray;
    DirectX::XMStoreFloat3(
      &ray.Origin,
      DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&Origin), m));
    DirectX::XMStoreFloat3(
      &ray.Direction,
      DirectX::XMVector3TransformNormal(DirectX::XMLoadFloat3(&Direction), m));
    return ray;
  }
};

}
}
