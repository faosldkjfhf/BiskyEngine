#pragma once

#include "DX12/DescriptorHeap.h"
#include <functional>

namespace Editor
{

struct HeapAllocator
{
  Owner<DX12::DescriptorHeap> Heap = nullptr;
  D3D12_DESCRIPTOR_HEAP_TYPE HeapType;
  D3D12_CPU_DESCRIPTOR_HANDLE HeapStartCPU;
  D3D12_GPU_DESCRIPTOR_HANDLE HeapStartGPU;
  UINT HeapHandleIncrement;
  std::vector<int> FreeIndices;

  inline void Create(ID3D12Device *device, Owner<DX12::DescriptorHeap> heap)
  {
    Heap = std::move(heap);
    D3D12_DESCRIPTOR_HEAP_DESC desc = Heap->Resource()->GetDesc();
    HeapType = desc.Type;
    HeapStartCPU = Heap->CPUDescriptorHandle();
    HeapStartGPU = Heap->GPUDescriptorHandle();
    HeapHandleIncrement = device->GetDescriptorHandleIncrementSize(HeapType);
    FreeIndices.reserve(desc.NumDescriptors);
    for (int n = desc.NumDescriptors; n > 0; n--)
    {
      FreeIndices.push_back(n - 1);
    }
  }

  inline void Destroy()
  {
    Heap = nullptr;
    FreeIndices.clear();
  }

  inline void Allocate(D3D12_CPU_DESCRIPTOR_HANDLE *outCPUDescHandle, D3D12_GPU_DESCRIPTOR_HANDLE *outGPUDescHandle)
  {
    if (FreeIndices.size() == 0)
    {
      throw std::runtime_error("no more free indices");
    }

    int idx = FreeIndices.back();
    FreeIndices.pop_back();
    outCPUDescHandle->ptr = HeapStartCPU.ptr + (idx * HeapHandleIncrement);
    outGPUDescHandle->ptr = HeapStartGPU.ptr + (idx * HeapHandleIncrement);
  }

  inline void Free(D3D12_CPU_DESCRIPTOR_HANDLE outCPUDescHandle, D3D12_GPU_DESCRIPTOR_HANDLE outGPUDescHandle)
  {
    int cpuIdx = (int)((outCPUDescHandle.ptr - HeapStartCPU.ptr) / HeapHandleIncrement);
    int gpuIdx = (int)((outGPUDescHandle.ptr - HeapStartGPU.ptr) / HeapHandleIncrement);
    if (cpuIdx != gpuIdx)
    {
      throw std::runtime_error("cpuIdx != gpuIdx");
    }

    FreeIndices.push_back(cpuIdx);
  }
};

class GUI
{
public:
  void Init();
  void Shutdown();
  void NewFrame(std::function<void()> &&function);
  void Draw(ID3D12GraphicsCommandList *gCmdList);

  GUI(const GUI &) = delete;
  const GUI &operator=(const GUI &) = delete;
  inline static GUI &Get()
  {
    static GUI instance;
    return instance;
  }

private:
  GUI() = default;
};

} // namespace Editor