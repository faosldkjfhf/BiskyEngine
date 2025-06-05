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
    explicit ForwardRenderer(gfx::Window *const window, gfx::Device *const backend);
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

  public:
  private:
    void initRootSignatures();
    void initPipelineStateObjects();

  private:
    gfx::Device *const m_backend;
};

} // namespace bisky::renderer