#pragma once

#include "Common.hpp"
#include "Graphics/Texture.hpp"

namespace bisky::scene
{

struct Material
{
    dx::XMFLOAT3 diffuse;
    float          metallic;
    float          roughness;
    float          ambientOcclusion;
    gfx::Texture  *diffuseTexture;
    gfx::Texture  *normalTexture;
    gfx::Texture  *metallicRoughnessTexture;
    gfx::Texture  *ambientOccusionTexture;
};

} // namespace bisky::scene