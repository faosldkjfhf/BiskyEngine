#pragma once

#include "Graphics/GraphicsCommandList.hpp"
#include "Graphics/ResourceUpload.hpp"
#include "Graphics/Utilities.hpp"
#include "Scene/Mesh.hpp"

namespace bisky::scene
{

struct ScreenQuad
{
    struct Vertex
    {
        math::XMFLOAT3 Position;
        math::XMFLOAT2 TexCoord;
    };

    inline static std::unique_ptr<Mesh> mesh(gfx::Device *const device)
    {
        gfx::ResourceUpload upload(device);
        upload.Begin();

        std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();
        mesh->name                 = "ScreenQuad";

        constexpr ScreenQuad::Vertex vertices[] = {
            {{-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{1.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
            {{1.0f, -1.0f, 0.0f}, {1.0f, 1.0f}}
        };
        constexpr uint32_t indices[] = {0, 1, 2, 2, 1, 3};

        mesh->vertexBufferByteSize = sizeof(vertices);
        mesh->vertexByteStride     = sizeof(ScreenQuad::Vertex);
        mesh->indexBufferByteSize  = sizeof(indices);
        mesh->indexFormat          = DXGI_FORMAT_R32_UINT;

        Submesh submesh{};
        submesh.baseVertexLocation = 0;
        submesh.startIndexLocation = 0;
        submesh.indexCount         = _countof(indices);
        mesh->submeshes.push_back(submesh);

        // Create the vertex and index buffers
        mesh->vertexBuffer = device->createUploadBuffer(mesh->vertexBufferByteSize, (void *)vertices);
        mesh->indexBuffer  = device->createUploadBuffer(mesh->indexBufferByteSize, (void *)indices);

        // Allocate a SRV Descriptor
        mesh->vertexBuffer->srvDescriptor = device->getCbvSrvUavHeap()->allocate();

        // Create the SRV
        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {
            .Format                  = DXGI_FORMAT_UNKNOWN,
            .ViewDimension           = D3D12_SRV_DIMENSION_BUFFER,
            .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
            .Buffer =
                {
                    .FirstElement        = 0,
                    .NumElements         = _countof(vertices),
                    .StructureByteStride = sizeof(ScreenQuad::Vertex),
                    .Flags               = D3D12_BUFFER_SRV_FLAG_NONE,
                },
        };
        device->createShaderResourceView(mesh->vertexBuffer.get(), &desc);

        auto finish = upload.Finish();
        finish.wait();

        return std::move(mesh);
    }
};

} // namespace bisky::scene