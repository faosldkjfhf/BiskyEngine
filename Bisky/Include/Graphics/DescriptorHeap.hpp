#pragma once

#include "Common.hpp"
#include "Descriptor.hpp"

namespace bisky::gfx
{

/*
 * A less verbose way to describe descriptor heap type.
 */
enum class DescriptorType
{
    CbvSrvUav = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
    Rtv       = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
    Dsv       = D3D12_DESCRIPTOR_HEAP_TYPE_DSV
};

/*
 * A less verbose way to describe descriptor heap flags.
 */
enum class DescriptorFlags
{
    None          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
    ShaderVisible = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
};

/*
 * A wrapper around a descriptor heap.
 *
 * TODO:
 * 1. Allow deleting descriptors.
 * 2. Allow resizing so if descriptors goes over it reallocates everything.
 */
class DescriptorHeap
{
  public:
    /*
     * Initializes a descriptor heap with the given type, flags, and number of descriptors.
     * Also fetches the CPU heap start handle, GPU heap start handle, and descriptor size.
     *
     * @param device The device to use.
     * @param type The type of descriptor heap.
     * @param numDescriptors The maximum number of descriptors in the heap.
     * @param flags The flag to set.
     */
    explicit DescriptorHeap(ID3D12Device10 *device, DescriptorType type, uint32_t numDescriptors,
                            DescriptorFlags flags = DescriptorFlags::None);

    /*
     * Resets the descriptor heap.
     */
    ~DescriptorHeap();

    DescriptorHeap(const DescriptorHeap &)                    = delete;
    const DescriptorHeap &operator=(const DescriptorHeap &)   = delete;
    DescriptorHeap(const DescriptorHeap &&)                   = delete;
    const DescriptorHeap &&operator=(const DescriptorHeap &&) = delete;

  public:
    /*
     * Gets the inner descriptor heap.
     *
     * @return An unmodifiable pointer to the inner descriptor heap.
     */
    ID3D12DescriptorHeap *const getHeap() const;

    /*
     * Returns a handle to the CPU heap start.
     *
     * @return An unmodifiable handle to the CPU heap start.
     */
    const D3D12_CPU_DESCRIPTOR_HANDLE &getCpuHeapStart() const;

    /*
     * Returns a handle to the GPU heap start.
     *
     * @return An unmodifiable handle to the GPU heap start.
     */
    const D3D12_GPU_DESCRIPTOR_HANDLE &getGpuHeapStart() const;

    /*
     * Gets the descriptor size of this type of heap.
     *
     * @return The descriptor size of this heap.
     */
    uint32_t getDescriptorSize() const;

  public:
    /*
     * Increments the heap index tracker and returns the latest value.
     * If the descriptor limit is exceeded, returns the last value.
     *
     * @return The descriptor heap index you can use.
     */
    Descriptor allocate();

  private:
    wrl::ComPtr<ID3D12DescriptorHeap> m_heap;
    D3D12_CPU_DESCRIPTOR_HANDLE       m_cpuHeapStart;
    D3D12_GPU_DESCRIPTOR_HANDLE       m_gpuHeapStart;
    uint32_t                          m_descriptorSize = 0u;
    uint32_t                          m_capacity       = 0u;
    uint32_t                          m_at             = 0u;
};

} // namespace bisky::gfx