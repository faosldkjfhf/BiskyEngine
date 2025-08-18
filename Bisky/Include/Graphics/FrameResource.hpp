#pragma once

#include "Graphics/Allocator.hpp"
#include "Graphics/Buffer.hpp"
#include "Graphics/GraphicsCommandList.hpp"

namespace bisky::gfx
{

class Device;
class Window;

/*
 * Stores items that vary per frame.
 */
struct FrameResource
{
    explicit FrameResource(Device *const device, Window *const window);

    auto resize(Device *const device, Window *const window) -> void;

    std::unique_ptr<GraphicsCommandList> graphicsCommandList = nullptr;
    std::unique_ptr<Allocator>           resourceAllocator   = nullptr;
    std::unique_ptr<Buffer>              sceneBuffer         = nullptr;
    std::unique_ptr<Texture>             bloomTexture        = nullptr;
    uint64_t                             fenceValue          = 0u;
};

} // namespace bisky::gfx