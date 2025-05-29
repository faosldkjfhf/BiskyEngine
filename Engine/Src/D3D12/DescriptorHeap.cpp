#include "Common.h"

#include "D3D12/Context.h"
#include "D3D12/DescriptorHeap.h"

namespace D3D12
{

DescriptorHeap::DescriptorHeap(DescriptorType type, UINT numDescriptors, DescriptorFlags flags)
    : mNumDescriptors(numDescriptors), mCurrentDescriptor(0)
{
  D3D12_DESCRIPTOR_HEAP_DESC hd{};
  hd.Type = static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(type);
  hd.NumDescriptors = numDescriptors;
  hd.Flags = static_cast<D3D12_DESCRIPTOR_HEAP_FLAGS>(flags);
  hd.NodeMask = 0;
  Context::Get().Device()->CreateDescriptorHeap(&hd, IID_PPV_ARGS(&mHeap));
}

UINT DescriptorHeap::AddDescriptor()
{
  return mCurrentDescriptor++;
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::CPUDescriptorHandle() const
{
  return mHeap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GPUDescriptorHandle() const
{
  return mHeap->GetGPUDescriptorHandleForHeapStart();
}

} // namespace D3D12