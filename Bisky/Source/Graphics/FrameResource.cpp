#include "Common.hpp"

#include "Graphics/Allocator.hpp"
#include "Graphics/Constants.hpp"
#include "Graphics/Device.hpp"
#include "Graphics/FrameResource.hpp"

namespace bisky::gfx
{

FrameResource::FrameResource(Device *const device)
{
    UINT32 size = (sizeof(gfx::SceneBuffer) + 255) & -256;

    // ----- initialize resources -----
    graphicsCommandList = std::make_unique<gfx::GraphicsCommandList>(device);
    resourceAllocator   = std::make_unique<gfx::Allocator>(device, 64u * 1024u);
    sceneBuffer         = device->createUploadBuffer(size);
    fenceValue          = 0u;

    // ----- create constant buffer view -----
    sceneBuffer->cbvDescriptor          = device->getCbvSrvUavHeap()->allocate();
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbv = {
        .BufferLocation = sceneBuffer->resource->GetGPUVirtualAddress(),
        .SizeInBytes    = size,
    };
    device->getDevice()->CreateConstantBufferView(&cbv, sceneBuffer->cbvDescriptor.cpu);
}

} // namespace bisky::gfx