#pragma once

#include "Scene/RenderObject.hpp"

namespace bisky::core
{
struct FrameStats;
}

namespace bisky::gfx
{
class Device;
struct Texture;
} // namespace bisky::gfx

namespace bisky::scene
{
class Scene;
}

namespace bisky::renderer
{

class SkyboxRenderPass
{
  public:
    struct RenderResource
    {
        int vertexBufferIndex = -1;
        int textureIndex      = -1;
    };

    explicit SkyboxRenderPass(gfx::Device *const device);
    ~SkyboxRenderPass();

    SkyboxRenderPass(const SkyboxRenderPass &)                    = delete;
    const SkyboxRenderPass &operator=(const SkyboxRenderPass &)   = delete;
    SkyboxRenderPass(const SkyboxRenderPass &&)                   = delete;
    const SkyboxRenderPass &&operator=(const SkyboxRenderPass &&) = delete;

  public:
    void draw(gfx::FrameResource *const frameResource, scene::Scene *const scene, core::FrameStats *const frameStats);

  private:
    void initRootSignature();
    void initPipelineState();

  private:
    gfx::Device *const                   m_device;
    std::unique_ptr<scene::RenderObject> m_cube;
};

} // namespace bisky::renderer