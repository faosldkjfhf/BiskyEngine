#pragma once

#include "Common.hpp"

namespace bisky::gfx
{

class CommandList;

/*
 * This is a wrapper class around a command queue and fence.
 */
class CommandQueue
{
  public:
    /*
     * Initializes a command queue with the given type.
     * Also initializes the fence.
     *
     * @param device The device to use.
     * @param commandListType The type of command list this queue is for.
     */
    explicit CommandQueue(ID3D12Device10 *const   device,
                          D3D12_COMMAND_LIST_TYPE commandListType = D3D12_COMMAND_LIST_TYPE_DIRECT);

    /*
     * Resets all stored variables
     */
    ~CommandQueue();

    CommandQueue(const CommandQueue &)                    = delete;
    const CommandQueue &operator=(const CommandQueue &)   = delete;
    CommandQueue(const CommandQueue &&)                   = delete;
    const CommandQueue &&operator=(const CommandQueue &&) = delete;

  public: // Getter functions
    /*
     * Gets the inner command queue.
     *
     * @return An unmodifiable pointer to the command queue.
     */
    ID3D12CommandQueue *const getCommandQueue() const;

    /*
     * Gets the inner fence.
     *
     * @return An unmodifable pointer to the fence.
     */
    ID3D12Fence *const getFence() const;

    /*
     * Gets the last completed value of the fence.
     *
     * @return The last completed value of the fence.
     */
    uint64_t getCompletedValue() const;

  public: // Public methods
    /*
     * Closes and executes the given command lists.
     *
     * @param commandLists The lists to execute.
     */
    void executeCommandLists(const std::span<const CommandList *const> commandLists);

    /*
     * Signals the fence for the next value.
     *
     * @return The most recent fence value.
     */
    uint64_t signal();

    /*
     * Waits for the given fence value.
     *
     * @param fenceValue The fence value to wait for
     */
    void waitForFence(const uint64_t fenceValue) const;

    /*
     * Flushes the command queue by signaling fence and waiting for it.
     */
    void flush();

  private: // Private variables
    wrl::ComPtr<ID3D12CommandQueue> m_commandQueue;
    wrl::ComPtr<ID3D12Fence>        m_fence;
    uint64_t                        m_fenceValue = 0u;
};

} // namespace bisky::gfx