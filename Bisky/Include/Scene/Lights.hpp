#pragma once

#include "Common.hpp"

namespace bisky::scene
{

struct Light
{
    virtual ~Light() = default;

    math::XMFLOAT4 strength;
};

struct PointLight : public Light
{
    math::XMFLOAT4 position;
};

struct DirectionalLight : public Light
{
    math::XMFLOAT4 direction;
};

} // namespace bisky::scene