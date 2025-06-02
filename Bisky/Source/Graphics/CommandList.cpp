#include "Common.hpp"

#include "Graphics/Buffer.hpp"
#include "Graphics/CommandList.hpp"
#include "Graphics/Texture.hpp"

namespace bisky::gfx
{

void CommandList::addBarrier(Texture *texture, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter)
{
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource   = texture->resource.Get();
    barrier.Transition.StateBefore = stateBefore;
    barrier.Transition.StateAfter  = stateAfter;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    m_barriers.push_back(barrier);
}

void CommandList::addBarrier(Buffer *buffer, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter)
{
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource   = buffer->resource.Get();
    barrier.Transition.StateBefore = stateBefore;
    barrier.Transition.StateAfter  = stateAfter;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    m_barriers.push_back(barrier);
}

void CommandList::dispatchBarriers()
{
    m_commandList->ResourceBarrier(static_cast<UINT>(m_barriers.size()), m_barriers.data());
    m_barriers.clear();
}

} // namespace bisky::gfx