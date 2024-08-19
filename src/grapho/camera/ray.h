#include "../vertexlayout.h"
#include <cmath>

namespace grapho {
namespace camera {

struct Ray
{
  XMFLOAT3 Origin;
  XMFLOAT3 Direction;

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

  // Ray Transform(const DirectX::XMMATRIX& m) const;
};

} // namespace
} // namespace
