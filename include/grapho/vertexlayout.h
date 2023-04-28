#pragma once
#include <expected>
#include <string>
#include <vector>

namespace grapho {

enum class ValueType
{
  Float,
};

struct VertexId
{
  std::string semantic_name;
  uint32_t semantic_index;
  uint32_t slot;
};

struct VertexLayout
{
  VertexId id;
  ValueType type;
  uint32_t count;
  uint32_t offset;
  uint32_t stride;
  uint32_t divisor = 0;
};

}
