#pragma once

#include "Common.hpp"

namespace bisky::scene
{

struct Vertex
{
    dx::XMFLOAT3 position;
    dx::XMFLOAT3 normal;
    dx::XMFLOAT2 texCoord;
    dx::XMFLOAT4 tangent;
};

} // namespace bisky::scene