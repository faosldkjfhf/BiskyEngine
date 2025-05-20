#include "Common.h"

#include "Core/Logger.h"
#include "DX12/Context.h"

namespace DX12
{

Context::Error Context::Init()
{
  D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&mDevice));
  CreateDXGIFactory2(0, IID_PPV_ARGS(&mFactory));
  mDevice->CreateFence(mFenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence));

  mCbvSrvUavDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  mDsvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
  mRtvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  mSamplerDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

  D3D12_COMMAND_QUEUE_DESC queueDesc{};
  queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
  queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  queueDesc.NodeMask = 0;
  mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCmdQueue));
  mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCmdListAlloc));
  mDevice->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&mCmdList));

  mConstantBufferHeap = MakeOwner<DescriptorHeap>(DescriptorType::ConstantBuffer, 64, DescriptorFlags::ShaderVisible);

  LOG_INFO("D3D12 Initialized");
  return Error::None;
}

void Context::Shutdown()
{
  mConstantBufferHeap.reset();
  mCmdList.Reset();
  mCmdListAlloc.Reset();
  mCmdQueue.Reset();
  mFence.Reset();
  mFactory.Reset();
  mDevice.Reset();
}

ID3D12GraphicsCommandList10 *Context::ResetCommandList(ID3D12CommandAllocator *cmdListAlloc,
                                                       ID3D12PipelineState *pipelineState)
{
  if (cmdListAlloc == nullptr)
  {
    mCmdListAlloc->Reset();
    mCmdList->Reset(mCmdListAlloc.Get(), pipelineState);
  }
  else
  {
    cmdListAlloc->Reset();
    mCmdList->Reset(cmdListAlloc, pipelineState);
  }

  return mCmdList.Get();
}

void Context::ExecuteCommandList(ID3D12GraphicsCommandList10 *cmdList)
{
  if (SUCCEEDED(cmdList->Close()))
  {
    ID3D12CommandList *cmdLists[] = {cmdList};
    mCmdQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);
  }
}

void Context::FlushCommandQueue()
{
  mCmdQueue->Signal(mFence.Get(), ++mFenceValue);
  if (mFence->GetCompletedValue() < mFenceValue)
  {
    mFence->SetEventOnCompletion(mFenceValue, mFenceEvent);
    WaitForSingleObject(mFenceEvent, INFINITE);
    CloseHandle(mFenceEvent);
  }
}

void Context::WaitForFence(UINT64 fence)
{
  if (mFence->GetCompletedValue() < fence)
  {
    HANDLE handle = 0;
    Context::Get().Fence()->SetEventOnCompletion(fence, handle);
    WaitForSingleObject(handle, INFINITE);
    CloseHandle(handle);
  }
}

UINT64 Context::AdvanceFence()
{
  return ++mFenceValue;
}

} // namespace DX12