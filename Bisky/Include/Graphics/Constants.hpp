#pragma once

#include "Common.hpp"
#include "Scene/Lights.hpp"

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
    dx::XMFLOAT4X4 view;
    dx::XMFLOAT4X4 projection;
    dx::XMFLOAT4X4 viewProjection;
    dx::XMFLOAT4   viewPosition;
};

/*
 * This holds values that change per object.
 */
struct ObjectBuffer
{
    dx::XMFLOAT4X4 world;
    dx::XMFLOAT4X4 inverseWorld;
    dx::XMFLOAT4X4 tranposeInverseWorld;
};

struct LightBuffer
{
    scene::PointLight lights[10];
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
    int32_t sceneBufferIndex              = -1;
    int32_t diffuseTextureIndex           = -1;
    int32_t metallicRoughnessTextureIndex = -1;
};

} // namespace bisky::gfx