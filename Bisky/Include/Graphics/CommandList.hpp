#pragma once

#include "Common.hpp"

namespace bisky::gfx
{

struct Texture;
struct Buffer;

/*
 * This is a wrapper around a basic command list and command allocator.
 *
 * TODO: None right now
 */
class CommandList
{
  public:
    /*
     * Resets the command allocator and list.
     * Differs per command list type.
     */
    virtual void reset() = 0;

    /*
     * Gets the D3D12 command list.
     *
     * @return The stored command list.
     */
    ID3D12GraphicsCommandList10 *const getCommandList() const
    {
        return m_commandList.Get();
    }

    // supposedly it's better to batch out barriers all out once rather than one at a time
    // add barriers until you need to dispatch
    // call dispatch, dispatch will reset the barriers after the call

    /*
     * Adds a barrier to be dispatched.
     * This adds a transition for a resource between two states.
     *
     * @param pResource The resource to transition.
     * @param stateBefore The initial state.
     * @param stateAfter The state to transition to.
     */
    void addBarrier(Texture *texture, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter);
    void addBarrier(Buffer *buffer, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter);

    /*
     * Dispatches stored barriers and clears the vector afterward.
     */
    void dispatchBarriers();

  protected:
    explicit CommandList() = default;
    virtual ~CommandList() = default;

    wrl::ComPtr<ID3D12GraphicsCommandList10> m_commandList;
    wrl::ComPtr<ID3D12CommandAllocator>      m_commandAllocator;
    std::vector<D3D12_RESOURCE_BARRIER>      m_barriers;
};

} // namespace bisky::gfx