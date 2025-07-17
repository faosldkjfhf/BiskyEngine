#pragma once

#include "Graphics/Texture.hpp"
#include <DirectXMath.h>

namespace bisky::scene
{

struct Material
{
    DirectX::XMFLOAT3 diffuse;
    float             metallic;
    float             roughness;
    float             ambientOcclusion;
    gfx::Texture     *diffuseTexture;
    gfx::Texture     *normalTexture;
    gfx::Texture     *metallicRoughnessTexture;
    gfx::Texture     *ambientOccusionTexture;
};

} // namespace bisky::scene