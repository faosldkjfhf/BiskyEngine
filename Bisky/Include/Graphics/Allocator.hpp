#pragma once

#include "Common.hpp"
#include "Graphics/Buffer.hpp"

namespace bisky::gfx
{

class Device;

/*
 * An allocation that is returned by the allocator
 */
struct Allocation
{
    ID3D12Resource *const     resource; // pointer to the actual resource
    void                     *cpuBase;  // mapped data (for updating)
    D3D12_GPU_VIRTUAL_ADDRESS gpuBase;  // gpu address
    uint32_t                  offset;   // offset
    uint32_t                  size;     // size of the allocation
};

/*
 * An arena allocator mainly meant for upload buffers
 */
struct Allocator
{
    std::unique_ptr<Buffer>   buffer;   // the buffer to allocate to
    char                     *cpuBase;  // pointer to the start of the mapped data
    D3D12_GPU_VIRTUAL_ADDRESS gpuBase;  // pointer to the start of gpu address
    uint32_t                  at;       // current offset in the memory
    uint32_t                  capacity; // total memory allocated

    Allocator(Device *const device, uint32_t size);
    ~Allocator();

    Allocator(const Allocator &)                    = delete;
    const Allocator &operator=(const Allocator &)   = delete;
    Allocator(const Allocator &&)                   = delete;
    const Allocator &&operator=(const Allocator &&) = delete;

    /*
     * Allocates the size aligned by align if possible.
     * Returns an Allocation struct with the necessary information.
     */
    Allocation allocate(uint32_t size, uint32_t align = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

    /*
     * Resets all allocations made by setting 'at' to 0u.
     * This should be called at the end of each frame.
     */
    void reset();
};

} // namespace bisky::gfx