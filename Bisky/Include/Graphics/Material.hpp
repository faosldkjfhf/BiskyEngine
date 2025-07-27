#pragma once

#include <DirectXMath.h>
#include <memory>

namespace bisky::gfx
{

struct Texture;

struct Material
{
    std::shared_ptr<gfx::Texture> diffuseTexture;
    std::shared_ptr<gfx::Texture> normalTexture;
    std::shared_ptr<gfx::Texture> metallicRoughnessTexture;
    std::shared_ptr<gfx::Texture> ambientOccusionTexture;
};

} // namespace bisky::gfx