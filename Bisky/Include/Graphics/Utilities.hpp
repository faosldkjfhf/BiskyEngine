#pragma once

#include "Graphics/Buffer.hpp"
#include "Graphics/Device.hpp"

namespace bisky::gfx
{

inline std::unique_ptr<Buffer> createBuffer(Device *const device, GraphicsCommandList *const cmdList,
                                            wrl::ComPtr<ID3DBlob> &data, Buffer *const uploadBuffer)
{
    std::unique_ptr<Buffer> buffer = std::make_unique<Buffer>();

    D3D12_HEAP_PROPERTIES hp{};
    hp.Type                 = D3D12_HEAP_TYPE_DEFAULT;
    hp.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    hp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    hp.VisibleNodeMask      = 0;
    hp.CreationNodeMask     = 0;

    D3D12_RESOURCE_DESC rd{};
    rd.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
    rd.Format             = DXGI_FORMAT_UNKNOWN;
    rd.Width              = data->GetBufferSize();
    rd.Height             = 1;
    rd.DepthOrArraySize   = 1;
    rd.MipLevels          = 1;
    rd.SampleDesc.Count   = 1;
    rd.SampleDesc.Quality = 0;
    rd.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    rd.Flags              = D3D12_RESOURCE_FLAG_NONE;

    device->getDevice()->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_COMMON, nullptr,
                                                 IID_PPV_ARGS(&buffer->resource));

    hp.Type = D3D12_HEAP_TYPE_UPLOAD;
    device->getDevice()->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_GENERIC_READ,
                                                 nullptr, IID_PPV_ARGS(&uploadBuffer->resource));

    void *upload;
    uploadBuffer->resource->Map(0, nullptr, &upload);
    memcpy(upload, data->GetBufferPointer(), data->GetBufferSize());
    uploadBuffer->resource->Unmap(0, nullptr);

    cmdList->addBarrier(buffer.get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
    cmdList->dispatchBarriers();

    cmdList->copyBufferRegion(uploadBuffer, buffer.get(), data->GetBufferSize());

    cmdList->addBarrier(buffer.get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
    cmdList->dispatchBarriers();

    return std::move(buffer);
}

inline uint32_t constantBufferByteSize(uint32_t size)
{
    return (size + 255) & (~255);
}

} // namespace bisky::gfx