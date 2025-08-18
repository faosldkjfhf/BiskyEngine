#include "Common.hpp"

#include "Graphics/Allocator.hpp"
#include "Graphics/Constants.hpp"
#include "Graphics/Device.hpp"
#include "Graphics/FrameResource.hpp"
#include "Graphics/Window.hpp"

namespace bisky::gfx
{

FrameResource::FrameResource(Device *const device, Window *const window)
{
    constexpr UINT32 size = (sizeof(gfx::SceneBuffer) + 255) & ~255;

    // ----- initialize resources -----
    graphicsCommandList = std::make_unique<gfx::GraphicsCommandList>(device);
    resourceAllocator   = std::make_unique<gfx::Allocator>(device, 64u * 1024u);
    sceneBuffer         = device->createUploadBuffer(size);
    bloomTexture        = device->createTexture2D(
        window->getWidth(), window->getHeight(), DXGI_FORMAT_R16G16B16A16_FLOAT,
        D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
    );
    fenceValue = 0u;

    // -- create uav, srv, rtv for bloomTexture
    device->createUnorderedAccessView(bloomTexture.get());
    device->createShaderResourceView(bloomTexture.get());
    device->createRenderTargetView(bloomTexture.get());

    // ----- create constant buffer view -----
    sceneBuffer->cbvDescriptor          = device->getCbvSrvUavHeap()->allocate();
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbv = {
        .BufferLocation = sceneBuffer->resource->GetGPUVirtualAddress(),
        .SizeInBytes    = size,
    };
    device->getDevice()->CreateConstantBufferView(&cbv, sceneBuffer->cbvDescriptor.cpu);
}

auto FrameResource::resize(Device *const device, Window *const window) -> void
{
    bloomTexture.reset();

    bloomTexture = device->createTexture2D(
        window->getWidth(), window->getHeight(), DXGI_FORMAT_R16G16B16A16_FLOAT,
        D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
    );
    device->createUnorderedAccessView(bloomTexture.get());
    device->createShaderResourceView(bloomTexture.get());
    device->createRenderTargetView(bloomTexture.get());
}

} // namespace bisky::gfx