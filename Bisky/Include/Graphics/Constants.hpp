#pragma once

#include "Common.hpp"

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
    math::XMFLOAT4X4 view;
    math::XMFLOAT4X4 projection;
    math::XMFLOAT4X4 viewProjection;
};

/*
 * This holds values that change per object.
 */
struct ObjectBuffer
{
    math::XMFLOAT4X4 world;
    math::XMFLOAT4X4 inverseWorld;
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
    int32_t sceneBufferIndex              = -1;
    int32_t diffuseTextureIndex           = -1;
    int32_t metallicRoughnessTextureIndex = -1;
};

} // namespace bisky::gfx