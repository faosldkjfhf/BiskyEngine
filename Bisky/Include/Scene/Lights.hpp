#pragma once

#include "Common.hpp"

namespace bisky::scene
{

struct Light
{
    math::XMFLOAT4 strength;
};

struct PointLight
{
    math::XMFLOAT4 position;
    math::XMFLOAT4 strength;
};

struct DirectionalLight : public Light
{
    math::XMFLOAT4 direction;
};

} // namespace bisky::scene