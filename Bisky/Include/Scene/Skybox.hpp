#pragma once

#include "Graphics/Texture.hpp"

namespace bisky::gfx
{
class Device;
}

namespace bisky::scene
{

/*
 * A wrapper around a texture cube.
 *
 * TODO: Add in support for changing out the texture.
 * TODO: Add in support for generating an HDR cubemap.
 */
class Skybox
{
  public:
    explicit Skybox(gfx::Device *const device, const std::string_view name);
    ~Skybox();

    Skybox(const Skybox &)                  = delete;
    const Skybox &operator=(const Skybox &) = delete;
    Skybox(const Skybox &&)                 = delete;

  public:
    gfx::Texture *const getTexture() const;

  private:
    gfx::Device *const            m_device;
    std::shared_ptr<gfx::Texture> m_skybox;
};

} // namespace bisky::scene