#include "Common.hpp"

#include "Graphics/DescriptorHeap.hpp"

namespace bisky::gfx
{

DescriptorHeap::DescriptorHeap(ID3D12Device10 *device, DescriptorType type, uint32_t numDescriptors,
                               DescriptorFlags flags)
    : m_capacity(numDescriptors)
{
    D3D12_DESCRIPTOR_HEAP_DESC desc{};
    desc.Flags          = static_cast<D3D12_DESCRIPTOR_HEAP_FLAGS>(flags);
    desc.Type           = static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(type);
    desc.NumDescriptors = numDescriptors;
    desc.NodeMask       = 0;
    device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_heap));

    m_descriptorSize = device->GetDescriptorHandleIncrementSize(static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(type));
    m_cpuHeapStart   = m_heap->GetCPUDescriptorHandleForHeapStart();

    if (flags == DescriptorFlags::ShaderVisible)
    {
        m_gpuHeapStart = m_heap->GetGPUDescriptorHandleForHeapStart();
    }
}

DescriptorHeap::~DescriptorHeap()
{
    m_heap.Reset();
}

ID3D12DescriptorHeap *const DescriptorHeap::getHeap() const
{
    return m_heap.Get();
}

const D3D12_CPU_DESCRIPTOR_HANDLE &DescriptorHeap::getCpuHeapStart() const
{
    return m_cpuHeapStart;
}

const D3D12_GPU_DESCRIPTOR_HANDLE &DescriptorHeap::getGpuHeapStart() const
{
    return m_gpuHeapStart;
}

uint32_t DescriptorHeap::getDescriptorSize() const
{
    return m_descriptorSize;
}

Descriptor DescriptorHeap::allocate()
{
    assert(m_at < m_capacity);

    uint32_t index = m_at++;

    Descriptor descriptor = {
        .cpu   = {m_cpuHeapStart.ptr + index * m_descriptorSize},
        .gpu   = {m_gpuHeapStart.ptr + index * m_descriptorSize},
        .index = static_cast<int32_t>(index),
    };

    return descriptor;
}

} // namespace bisky::gfx