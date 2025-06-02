#include "Common.hpp"

#include "Graphics/Allocator.hpp"
#include "Graphics/Device.hpp"

namespace bisky::gfx
{

Allocator::Allocator(Device *const device, uint32_t size)
{
    buffer = device->createUploadBuffer(size);

    void *mapped;
    buffer->resource->Map(0, nullptr, &mapped);

    cpuBase  = (char *)mapped;
    gpuBase  = buffer->resource->GetGPUVirtualAddress();
    at       = 0u;
    capacity = size;
}

Allocator::~Allocator()
{
    if (cpuBase)
    {
        buffer->resource->Unmap(0, nullptr);
    }

    buffer.reset();
}

Allocation Allocator::allocate(uint32_t size, uint32_t align)
{
    uint32_t alignedSize = (size + (align - 1)) & (-(int32_t)align);
    uint32_t aligned     = (at + (align - 1)) & (-(int32_t)align);

    Allocation allocation = {
        .resource = buffer->resource.Get(),
        .cpuBase  = cpuBase + aligned,
        .gpuBase  = gpuBase + aligned,
        .offset   = aligned,
        .size     = alignedSize,
    };

    at = aligned + size;

    return allocation;
}

void Allocator::reset()
{
    at = 0;
}

} // namespace bisky::gfx