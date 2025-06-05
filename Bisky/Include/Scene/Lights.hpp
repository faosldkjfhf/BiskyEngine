#pragma once

#include "Common.hpp"

namespace bisky::scene
{

struct Light
{
    dx::XMFLOAT4 strength;
};

struct PointLight
{
    dx::XMFLOAT4 position;
    dx::XMFLOAT4 strength;
};

struct DirectionalLight : public Light
{
    dx::XMFLOAT4 direction;
};

} // namespace bisky::scene