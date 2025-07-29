#pragma once

#include "Graphics/GraphicsPipeline.hpp"
#include <memory>

namespace bisky::gfx
{
class Device;
class Window;
struct Texture;
} // namespace bisky::gfx

namespace bisky::scene
{
class Scene;
}

namespace bisky::renderer
{

struct GBuffer
{
    std::unique_ptr<gfx::Texture> position;
    std::unique_ptr<gfx::Texture> normal;
    std::unique_ptr<gfx::Texture> albedo;
};

class DeferredRenderer
{
  public:
    struct RenderResource
    {
        int vertexBufferIndex;
        int diffuseMapIndex;
    };

    explicit DeferredRenderer(gfx::Device *const device, gfx::Window *const window);
    ~DeferredRenderer();

  public:
    auto geometryPass(scene::Scene *const scene, gfx::FrameResource *const frameResource) const -> void;

  public:
    auto getGBuffer() -> GBuffer *;

  private:
    std::unique_ptr<gfx::GraphicsPipeline> m_graphicsPipeline;
    gfx::Device *const                     m_device;
    GBuffer                                m_gBuffer;
};

} // namespace bisky::renderer