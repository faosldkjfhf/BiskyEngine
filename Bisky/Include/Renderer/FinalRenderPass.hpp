#pragma once

#include "Scene/RenderObject.hpp"

namespace bisky::gfx
{
class Device;
struct FrameResource;
struct Texture;
} // namespace bisky::gfx

namespace bisky::renderer
{

class FinalRenderPass
{
  public:
    struct RenderResource
    {
        int vertexBufferIndex = -1;
        int textureIndex      = -1;
    };

    explicit FinalRenderPass(gfx::Device *const device);
    ~FinalRenderPass();

    FinalRenderPass(const FinalRenderPass &)                    = delete;
    const FinalRenderPass &operator=(const FinalRenderPass &)   = delete;
    FinalRenderPass(const FinalRenderPass &&)                   = delete;
    const FinalRenderPass &&operator=(const FinalRenderPass &&) = delete;

  public:
    void draw(gfx::FrameResource *const frameResource, gfx::Texture *const renderTexture);

  private:
    void initRootSignature();
    void initPipelineState();

    gfx::Device *const                   m_device;
    std::unique_ptr<scene::RenderObject> m_screenQuad;
};

} // namespace bisky::renderer