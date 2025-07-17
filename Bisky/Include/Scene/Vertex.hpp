#pragma once

#include <DirectXMath.h>

namespace bisky::scene
{

struct Vertex
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT2 texCoord;
    DirectX::XMFLOAT4 tangent;
};

} // namespace bisky::scene