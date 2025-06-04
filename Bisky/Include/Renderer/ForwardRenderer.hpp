#pragma once

#include "Renderer/RenderLayer.hpp"

namespace bisky::core
{
struct FrameStats;
}

namespace bisky::gfx
{
class Device;
class Window;
struct FrameResource;
} // namespace bisky::gfx

namespace bisky::scene
{
class Scene;
}

namespace bisky::renderer
{

class ForwardRenderer
{
  public:
    explicit ForwardRenderer(
        gfx::Window *const window, gfx::Device *const backend,
        DXGI_FORMAT renderTextureFormat = DXGI_FORMAT_R16G16B16A16_FLOAT
    );
    ~ForwardRenderer();

    ForwardRenderer(const ForwardRenderer &)                    = delete;
    const ForwardRenderer &operator=(const ForwardRenderer &)   = delete;
    ForwardRenderer(const ForwardRenderer &&)                   = delete;
    const ForwardRenderer &&operator=(const ForwardRenderer &&) = delete;

  public:
    void draw(
        const RenderLayer &renderLayer, gfx::FrameResource *frameResource, const scene::Scene *const scene,
        core::FrameStats *const frameStats
    );
    void                resize(uint32_t width, uint32_t height);
    gfx::Texture *const getRenderTexture(uint32_t index) const;

  private:
    void initRootSignatures();
    void initPipelineStateObjects();
    void initRenderResources(uint32_t width, uint32_t height);

  private:
    gfx::Device *const m_backend;

    std::array<std::unique_ptr<gfx::Texture>, gfx::Device::FramesInFlight> m_renderTextures;
    std::array<gfx::Descriptor, gfx::Device::FramesInFlight>               m_renderTextureSrvs;
    std::array<gfx::Descriptor, gfx::Device::FramesInFlight>               m_renderTextureRtvs;
    DXGI_FORMAT m_renderTextureFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
};

} // namespace bisky::renderer