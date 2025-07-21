#pragma once

#include "Graphics/Texture.hpp"
#include <DirectXMath.h>

namespace bisky::scene
{

struct Material
{
    DirectX::XMFLOAT3             diffuse;
    float                         metallic;
    float                         roughness;
    float                         ambientOcclusion;
    std::shared_ptr<gfx::Texture> diffuseTexture;
    std::shared_ptr<gfx::Texture> normalTexture;
    std::shared_ptr<gfx::Texture> metallicRoughnessTexture;
    std::shared_ptr<gfx::Texture> ambientOccusionTexture;
};

} // namespace bisky::scene