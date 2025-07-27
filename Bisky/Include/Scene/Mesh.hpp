#pragma once

#include "Graphics/Buffer.hpp"
#include "Graphics/Descriptor.hpp"

namespace bisky::gfx
{
struct Material;
}

namespace bisky::scene
{

/*
 * A Submesh contains the draw calls for our mesh.
 * One mesh can be made up of many small submeshes.
 */
struct Submesh
{
    UINT32                         baseVertexLocation;
    UINT32                         startIndexLocation;
    UINT32                         indexCount;
    std::shared_ptr<gfx::Material> material;
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