#pragma once

#include "Graphics/ComputePipeline.hpp"
#include <d3d12.h>
#include <wrl.h>

namespace bisky::gfx
{
class Device;
class Window;
} // namespace bisky::gfx

namespace bisky::renderer
{

class BloomComputePass
{
  public:
    struct RenderResource
    {
        INT32 sourceImageIndex = -1;
        INT32 outputImageIndex = -1;
    };

    explicit BloomComputePass(gfx::Device *const device);

  public:
    void draw(gfx::Window *const window, gfx::FrameResource *const frameResource) const;

  private:
    gfx::Device *const                    m_device;
    std::unique_ptr<gfx::ComputePipeline> m_computePipeline;
};

} // namespace bisky::renderer