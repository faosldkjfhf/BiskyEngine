#pragma once

#include "Graphics/Allocator.hpp"
#include "Graphics/Buffer.hpp"
#include "Graphics/GraphicsCommandList.hpp"

namespace bisky::gfx
{

/*
 * Stores items that vary per frame.
 */
struct FrameResource
{
    std::unique_ptr<GraphicsCommandList> graphicsCommandList = nullptr;
    std::unique_ptr<Allocator>           resourceAllocator   = nullptr;
    uint64_t                             fenceValue          = 0u;
};

} // namespace bisky::gfx