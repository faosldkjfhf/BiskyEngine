#pragma once

#include "Graphics/Buffer.hpp"
#include "Graphics/Descriptor.hpp"

namespace bisky::scene
{

struct Material;

/*
 * A Submesh contains the draw calls for our mesh.
 * One mesh can be made up of many small submeshes.
 */
struct Submesh
{
    uint32_t  baseVertexLocation;
    uint32_t  startIndexLocation;
    uint32_t  indexCount;
    Material *material;
};

/*
 * Represents a Mesh.
 */
struct Mesh
{
    std::string                  name;
    uint32_t                     vertexBufferByteSize;
    uint32_t                     vertexByteStride;
    uint32_t                     indexBufferByteSize;
    DXGI_FORMAT                  indexFormat;
    std::unique_ptr<gfx::Buffer> vertexBuffer;
    std::unique_ptr<gfx::Buffer> indexBuffer;
    std::vector<Submesh>         submeshes;
};

} // namespace bisky::scene