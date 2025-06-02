#pragma once

#include "Common.hpp"
#include "Graphics/Descriptor.hpp"

namespace bisky::gfx
{

/*
 * A less verbose way of defining texture type.
 */
enum class TextureType
{
    Texture2D = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
    Texture3D = D3D12_RESOURCE_DIMENSION_TEXTURE3D,
};

/*
 * A struc to hold image data for textures.
 */
struct ImageData
{
    int         width;
    int         height;
    int         channelCount;
    DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
};

/*
 * A wrapper class around a D3D12 Texture Resource
 */
struct Texture
{
    static int32_t GetSrvIndex(const Texture *const texture);

    wrl::ComPtr<ID3D12Resource> resource;
    gfx::Descriptor             srvDescriptor = {};
    gfx::Descriptor             uavDescriptor = {};
    gfx::Descriptor             rtvDescriptor = {};
    gfx::Descriptor             dsvDescriptor = {};
};

} // namespace bisky::gfx