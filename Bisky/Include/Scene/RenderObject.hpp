#pragma once

#include "Graphics/Device.hpp"
#include "Graphics/Transform.hpp"

namespace bisky::gfx
{
struct Texture;
}

namespace bisky::scene
{

struct Material;
struct Mesh;

/*
 * An object that is going to be rendered.
 */
struct RenderObject
{
    std::string                     name              = "RenderObject";                      // The name of the object
    Mesh                           *mesh              = nullptr;                             // The mesh for the object
    D3D12_PRIMITIVE_TOPOLOGY        primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; // the topology type
    uint32_t                        numFramesDirty    = gfx::Device::FramesInFlight; // number of frames to update
    std::unique_ptr<gfx::Transform> transform         = std::make_unique<gfx::Transform>(); // the transform
};

} // namespace bisky::scene