#pragma once

#include <memory>

namespace bisky::gfx
{
class Device;
class Window;
struct Texture;
} // namespace bisky::gfx

namespace bisky::renderer
{

struct GBuffer
{
    std::unique_ptr<gfx::Texture> position;
    std::unique_ptr<gfx::Texture> normals;
    std::unique_ptr<gfx::Texture> albedo;
    std::unique_ptr<gfx::Texture> specular;
};

class DeferredRenderer
{
  public:
    explicit DeferredRenderer(gfx::Device *const device, gfx::Window *const window);
    ~DeferredRenderer();

  public:
  private:
    gfx::Device *const m_device;
    GBuffer            m_gBuffer;
};

} // namespace bisky::renderer