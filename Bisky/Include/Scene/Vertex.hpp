#pragma once

#include "Common.hpp"

namespace bisky::scene
{

struct Vertex
{
    math::XMFLOAT3 position;
    math::XMFLOAT3 normal;
    math::XMFLOAT2 texCoord;
    math::XMFLOAT4 tangent;
};

} // namespace bisky::scene