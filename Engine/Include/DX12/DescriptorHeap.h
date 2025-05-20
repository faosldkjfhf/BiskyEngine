#pragma once

#include "Common.h"

namespace DX12
{

enum DescriptorType
{
  ConstantBuffer = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
  ShaderResource = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
  UnorderedAccess = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
  DepthStencil = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
  RenderTarget = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
  Sampler = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
};

enum DescriptorFlags
{
  None = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
  ShaderVisible = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
};

class DescriptorHeap
{
public:
  DescriptorHeap(DescriptorType type, UINT numDescriptors, DescriptorFlags flags);

  UINT AddDescriptor();
  D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptorHandle() const;
  D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptorHandle() const;

  inline ID3D12DescriptorHeap *Resource()
  {
    return mHeap.Get();
  }

private:
  ComPtr<ID3D12DescriptorHeap> mHeap;
  UINT mNumDescriptors;
  UINT mCurrentDescriptor;
};

} // namespace DX12
