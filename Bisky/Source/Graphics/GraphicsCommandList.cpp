#include "Common.hpp"

#include "Graphics/Buffer.hpp"
#include "Graphics/Device.hpp"
#include "Graphics/GraphicsCommandList.hpp"

namespace bisky::gfx
{

GraphicsCommandList::GraphicsCommandList(Device *device) : m_device(*device)
{
    device->getDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));
    device->getDevice()->CreateCommandList1(
        0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_commandList)
    );
}

void GraphicsCommandList::reset()
{
    m_commandAllocator->Reset();
    m_commandList->Reset(m_commandAllocator.Get(), nullptr);
}

void GraphicsCommandList::clearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, float color[4]) const
{
    m_commandList->ClearRenderTargetView(renderTargetView, color, 0, nullptr);
}

void GraphicsCommandList::clearRenderTargetViews(
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> const &renderTargetViews, float color[4]
) const
{
    for (auto const &rtv : renderTargetViews)
    {
        m_commandList->ClearRenderTargetView(rtv, color, 0, nullptr);
    }
}

void GraphicsCommandList::clearDepthStencilView(
    D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, float depth, uint32_t stencil
) const
{
    m_commandList->ClearDepthStencilView(
        depthStencilView, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, depth, stencil, 0, nullptr
    );
}

void GraphicsCommandList::setViewport(const D3D12_VIEWPORT &viewport) const
{
    m_commandList->RSSetViewports(1, &viewport);
}

void GraphicsCommandList::setScissorRect(const D3D12_RECT &scissor) const
{
    m_commandList->RSSetScissorRects(1, &scissor);
}

void GraphicsCommandList::setRenderTargets(const D3D12_CPU_DESCRIPTOR_HANDLE &renderTarget) const
{
    m_commandList->OMSetRenderTargets(1, &renderTarget, false, nullptr);
}

void GraphicsCommandList::setRenderTargets(
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> const &renderTargets, D3D12_CPU_DESCRIPTOR_HANDLE const &depthStencilView
) const
{
    m_commandList->OMSetRenderTargets(
        static_cast<UINT>(renderTargets.size()), renderTargets.data(), FALSE, &depthStencilView
    );
}

void GraphicsCommandList::setRenderTargets(std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> const &renderTargets) const
{
    m_commandList->OMSetRenderTargets(static_cast<UINT>(renderTargets.size()), renderTargets.data(), FALSE, nullptr);
}

void GraphicsCommandList::setRenderTargets(
    const D3D12_CPU_DESCRIPTOR_HANDLE &renderTarget, const D3D12_CPU_DESCRIPTOR_HANDLE &depthStencilView
) const
{
    m_commandList->OMSetRenderTargets(1, &renderTarget, false, &depthStencilView);
}

void GraphicsCommandList::copyBufferRegion(Buffer *const src, Buffer *const dst, size_t bufferSize) const
{
    m_commandList->CopyBufferRegion(dst->resource.Get(), 0, src->resource.Get(), 0, bufferSize);
}

void GraphicsCommandList::copyTextureRegion(Buffer *const src, Texture *const dst, const ImageData &imageData) const
{
    D3D12_TEXTURE_COPY_LOCATION copySrc = {
        .pResource = src->resource.Get(),
        .Type      = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
        .PlacedFootprint =
            {
                .Offset = 0u,
                .Footprint =
                    {
                        .Format   = imageData.format,
                        .Width    = static_cast<UINT>(imageData.width),
                        .Height   = static_cast<UINT>(imageData.height),
                        .Depth    = 1u,
                        .RowPitch = static_cast<UINT>(imageData.width * 4u),
                    },
            },
    };

    D3D12_TEXTURE_COPY_LOCATION copyDst = {
        .pResource        = dst->resource.Get(),
        .Type             = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
        .SubresourceIndex = 0u,
    };

    D3D12_BOX box = {
        .left   = 0u,
        .top    = 0u,
        .front  = 0u,
        .right  = static_cast<UINT>(imageData.width),
        .bottom = static_cast<UINT>(imageData.height),
        .back   = 1u
    };

    m_commandList->CopyTextureRegion(&copyDst, 0, 0, 0, &copySrc, &box);
}

void GraphicsCommandList::setDescriptorHeaps(const std::span<const DescriptorHeap *const> &descriptorHeaps) const
{
    std::vector<ID3D12DescriptorHeap *> heaps(descriptorHeaps.size());
    for (size_t i = 0; i < descriptorHeaps.size(); i++)
    {
        heaps[i] = descriptorHeaps[i]->getHeap();
    }

    m_commandList->SetDescriptorHeaps(static_cast<UINT>(heaps.size()), heaps.data());
}

void GraphicsCommandList::setPipelineState(gfx::PipelineState *const pipelineState) const
{
    if (pipelineState)
    {
        m_commandList->SetPipelineState(pipelineState->getPipelineState());
    }
}

void GraphicsCommandList::setPipelineState(ID3D12PipelineState *const pipelineState) const
{
    if (pipelineState)
    {
        m_commandList->SetPipelineState(pipelineState);
    }
}

void GraphicsCommandList::setRootSignature(ID3D12RootSignature *const rootSignature) const
{
    if (rootSignature == nullptr)
    {
        LOG_ERROR("No root signature");
    }
    m_commandList->SetGraphicsRootSignature(rootSignature);
}

void GraphicsCommandList::setComputeRootSignature(ID3D12RootSignature *const rootSignature) const
{
    m_commandList->SetComputeRootSignature(rootSignature);
}

void GraphicsCommandList::setIndexBuffer(const IndexBufferView &indexBufferView) const
{
    D3D12_INDEX_BUFFER_VIEW ibv{};
    ibv.BufferLocation = indexBufferView.bufferLocation;
    ibv.SizeInBytes    = indexBufferView.sizeInBytes;
    ibv.Format         = indexBufferView.format;
    m_commandList->IASetIndexBuffer(&ibv);
}

void GraphicsCommandList::setPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY topology) const
{
    m_commandList->IASetPrimitiveTopology(topology);
}

void GraphicsCommandList::drawIndexedInstanced(const scene::Submesh &submesh) const
{
    m_commandList->DrawIndexedInstanced(
        submesh.indexCount, 1, submesh.startIndexLocation, submesh.baseVertexLocation, 0
    );
}

void GraphicsCommandList::drawIndexedInstanced(const UINT32 indexCount, const UINT32 instanceCount) const
{
}

void GraphicsCommandList::setConstantBufferView(uint32_t index, D3D12_GPU_VIRTUAL_ADDRESS handle) const
{
    m_commandList->SetGraphicsRootConstantBufferView(index, handle);
}

void GraphicsCommandList::set32BitConstants(uint32_t index, uint32_t numValues, void *data) const
{
    m_commandList->SetGraphicsRoot32BitConstants(index, numValues, data, 0u);
}

void GraphicsCommandList::dispatch(UINT x, UINT y, UINT z) const
{
    m_commandList->Dispatch(x, y, z);
}

} // namespace bisky::gfx