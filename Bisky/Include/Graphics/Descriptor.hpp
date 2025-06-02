#pragma once

#include <cmath>
#include <d3d12.h>

namespace bisky::gfx
{

/*
 * A wrapper around a descriptor returned from a descriptor heap.
 * Contains the internal CPU handle, GPU handle, and
 * also its index on its respective descriptor heap.
 *
 * If index is -1, then that means that this doesn't exist.
 */
struct Descriptor
{
    D3D12_CPU_DESCRIPTOR_HANDLE cpu{0};
    D3D12_GPU_DESCRIPTOR_HANDLE gpu{0};
    int32_t                     index{-1};
};

} // namespace bisky::gfx