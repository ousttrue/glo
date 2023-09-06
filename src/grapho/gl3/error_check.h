#pragma once
#include <functional>
#include <optional>

namespace grapho {
namespace gl3 {
std::optional<const char*>
TryGetError();

inline void
CheckAndPrintError(const std::function<void(const char*)>& print)
{
  while (auto error = TryGetError()) {
    print(*error);
  }
}

} // namespace
} // namespace
