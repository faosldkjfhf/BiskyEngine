#include "Common.hpp"

#include "Graphics/CommandList.hpp"
#include "Graphics/CommandQueue.hpp"

namespace bisky::gfx
{

CommandQueue::CommandQueue(ID3D12Device10 *const device, D3D12_COMMAND_LIST_TYPE commandListType)
{
    D3D12_COMMAND_QUEUE_DESC cq{};
    cq.Flags    = D3D12_COMMAND_QUEUE_FLAG_NONE;
    cq.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
    cq.Type     = commandListType;
    cq.NodeMask = 0;
    device->CreateCommandQueue(&cq, IID_PPV_ARGS(&m_commandQueue));
    device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
    LOG_VERBOSE("Command Queue and Fence created");
}

CommandQueue::~CommandQueue()
{
    m_fence.Reset();
    m_commandQueue.Reset();
}

ID3D12CommandQueue *const CommandQueue::getCommandQueue() const
{
    return m_commandQueue.Get();
}

ID3D12Fence *const CommandQueue::getFence() const
{
    return m_fence.Get();
}

uint64_t CommandQueue::getCompletedValue() const
{
    return m_fence->GetCompletedValue();
}

void CommandQueue::executeCommandLists(const std::span<const CommandList *const> commandLists)
{
    std::vector<ID3D12CommandList *> lists;
    lists.reserve(commandLists.size());
    for (auto &commandList : commandLists)
    {
        commandList->getCommandList()->Close();
        lists.emplace_back(commandList->getCommandList());
    }

    m_commandQueue->ExecuteCommandLists(static_cast<UINT>(lists.size()), lists.data());
}

uint64_t CommandQueue::signal()
{
    m_commandQueue->Signal(m_fence.Get(), ++m_fenceValue);
    return m_fenceValue;
}

void CommandQueue::waitForFence(const uint64_t fenceValue) const
{
    if (getCompletedValue() < fenceValue)
    {
        HANDLE handle = 0;
        m_fence->SetEventOnCompletion(fenceValue, handle);
        WaitForSingleObject(handle, INFINITE);
        CloseHandle(handle);
    }
}

void CommandQueue::flush()
{
    waitForFence(signal());
}

} // namespace bisky::gfx