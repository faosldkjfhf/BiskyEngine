#pragma once

#include <DirectXMath.h>

namespace bisky::scene
{

struct Light
{
    DirectX::XMFLOAT4 strength;
};

struct PointLight
{
    DirectX::XMFLOAT4 position;
    DirectX::XMFLOAT4 intensity;
    DirectX::XMFLOAT3 direction;
    int               type;
};

} // namespace bisky::scene