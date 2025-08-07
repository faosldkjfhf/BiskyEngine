#pragma once

#include "Scene/Lights.hpp"
#include <DirectXMath.h>
#include <cstdint>

/*
 * A collection of types of constants.
 *
 * These are meant to be passed into the shaders.
 */
namespace bisky::gfx
{

/*
 * This holds values that are updated pretty infrequently.
 */
struct SceneBuffer
{
    DirectX::XMFLOAT4X4 view;
    DirectX::XMFLOAT4X4 projection;
    DirectX::XMFLOAT4X4 viewProjection;
    DirectX::XMFLOAT4   viewPosition;
};

/*
 * This holds values that change per object.
 */
struct ObjectBuffer
{
    DirectX::XMFLOAT4X4 world;
    DirectX::XMFLOAT4X4 inverseWorld;
    DirectX::XMFLOAT4X4 transposeInverseWorld;
};

struct LightBuffer
{
    scene::PointLight lights[32];
    uint32_t          numLights;
};

/*
 * This holds material indices.
 */
struct MaterialInfo
{
    int32_t diffuseTextureIndex           = -1;
    int32_t metallicRoughnessTextureIndex = -1;
};

/*
 * This holds values for accessing buffers for objects.
 */
struct RenderResource
{
    int32_t vertexBufferIndex             = -1;
    int32_t diffuseTextureIndex           = -1;
    int32_t metallicRoughnessTextureIndex = -1;
    int32_t normalTextureIndex            = -1;
};

} // namespace bisky::gfx