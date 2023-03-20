#pragma once
#include <functional>
#include <string_view>

namespace glo {
using ErrorHandler = std::function<void(std::string_view)>;
}
