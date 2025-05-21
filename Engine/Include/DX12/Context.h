#pragma once

#include "DX12/DescriptorHeap.h"

namespace DX12
{

class Context
{
public:
  enum Error
  {
    None = 0
  };

  Error Init();
  void Shutdown();
  ID3D12GraphicsCommandList10 *ResetCommandList(ID3D12CommandAllocator *cmdListAlloc = nullptr,
                                                ID3D12PipelineState *pipelineState = nullptr);
  void ExecuteCommandList(ID3D12GraphicsCommandList10 *cmdList);
  void FlushCommandQueue();
  void WaitForFence(UINT64 fence);
  UINT64 AdvanceFence();

private:
  void InitSamplers();

  ComPtr<ID3D12Device10> mDevice;
  ComPtr<IDXGIFactory7> mFactory;

  ComPtr<ID3D12CommandQueue> mCmdQueue;
  ComPtr<ID3D12CommandAllocator> mCmdListAlloc;
  ComPtr<ID3D12GraphicsCommandList10> mCmdList;

  ComPtr<ID3D12Fence> mFence;
  UINT64 mFenceValue = 0;
  HANDLE mFenceEvent = 0;

  UINT mCbvSrvUavDescriptorSize = 0;
  UINT mDsvDescriptorSize = 0;
  UINT mRtvDescriptorSize = 0;
  UINT mSamplerDescriptorSize = 0;

  Owner<DescriptorHeap> mConstantBufferHeap;
  Owner<DescriptorHeap> mShaderResourceHeap;
  std::vector<D3D12_STATIC_SAMPLER_DESC> mStaticSamplers;

public:
  Context(const Context &) = delete;
  const Context &operator=(const Context &) = delete;
  inline static Context &Get()
  {
    static Context instance;
    return instance;
  }

  inline const ComPtr<ID3D12Device10> &Device() const
  {
    return mDevice;
  }

  inline const ComPtr<IDXGIFactory7> &Factory() const
  {
    return mFactory;
  }

  inline const ComPtr<ID3D12Fence> &Fence() const
  {
    return mFence;
  }

  inline ComPtr<ID3D12CommandQueue> &CommandQueue()
  {
    return mCmdQueue;
  }

  inline UINT CbvSrvUavDescriptorSize() const
  {
    return mCbvSrvUavDescriptorSize;
  }

  inline UINT RtvDescriptorSize() const
  {
    return mRtvDescriptorSize;
  }

  inline UINT DsvDescriptorSize() const
  {
    return mDsvDescriptorSize;
  }

  inline UINT SamplerDescriptorSize() const
  {
    return mSamplerDescriptorSize;
  }

  inline DescriptorHeap *ConstantBufferHeap()
  {
    return mConstantBufferHeap.get();
  }

  inline DescriptorHeap *ShaderResourceHeap()
  {
    return mShaderResourceHeap.get();
  }

  inline const std::vector<D3D12_STATIC_SAMPLER_DESC> &StaticSamplers()
  {
    return mStaticSamplers;
  }

private:
  Context() = default;
};

} // namespace DX12
