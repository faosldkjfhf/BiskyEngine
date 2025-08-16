#pragma once

#include "Graphics/GraphicsPipeline.hpp"
#include "Scene/RenderObject.hpp"
#include <memory>

namespace bisky::core
{
struct FrameStats;
}

namespace bisky::gfx
{
class Device;
struct FrameResource;
} // namespace bisky::gfx

namespace bisky::scene
{
class Scene;
}

namespace bisky::renderer
{

class LightDebugRenderPass
{
  public:
    // -- render resources
    struct RenderResource
    {
        int32_t           vertexBufferIndex;
        DirectX::XMFLOAT3 color;
    };

    // -- instance look up table
    struct InstanceLUT
    {
        DirectX::XMFLOAT4X4 world;
    };

    explicit LightDebugRenderPass(gfx::Device *const device);

  public:
    void draw(gfx::FrameResource *const frameResource, scene::Scene *const scene, core::FrameStats *const frameStats)
        const;

  private:
    void initGraphicsPipeline();

  private:
    gfx::Device *const                     m_device;
    std::unique_ptr<gfx::GraphicsPipeline> m_graphicsPipeline;
    std::unique_ptr<scene::RenderObject>   m_sphere;
};

} // namespace bisky::renderer