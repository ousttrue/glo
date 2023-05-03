#pragma once
#include <DirectXMath.h>
#include <grapho/vertexlayout.h>
#include <memory>
#include <stdint.h>
#include <string_view>
#include <vector>

void
renderQuad();

unsigned int
loadTexture(std::string_view path);
