#pragma once

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

enum class RenderLayer
{
    Opaque,
    Transparent,
    Skybox
};

class ForwardRenderer
{
  public:
    explicit ForwardRenderer(gfx::Device *const backend);
    ~ForwardRenderer();

    ForwardRenderer(const ForwardRenderer &)                    = delete;
    const ForwardRenderer &operator=(const ForwardRenderer &)   = delete;
    ForwardRenderer(const ForwardRenderer &&)                   = delete;
    const ForwardRenderer &&operator=(const ForwardRenderer &&) = delete;

  public:
  public:
    void draw(const RenderLayer &renderLayer, gfx::FrameResource *frameResource, const scene::Scene *const scene);

  private:
    void initRootSignatures();
    void initPipelineStateObjects();

  private:
    gfx::Device *const m_backend;
};

} // namespace bisky::renderer