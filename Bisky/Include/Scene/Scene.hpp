#pragma once

#include "Graphics/Constants.hpp"
#include "Graphics/Device.hpp"
#include "Scene/Camera.hpp"
#include "Scene/Lights.hpp"
#include "Scene/RenderObject.hpp"

namespace bisky::core
{
class GameTimer;
}

namespace bisky::gfx
{
class Window;
} // namespace bisky::gfx

/*
 * TODO:
 * 1. Scene should hold textures
 * 2. Scene should hold geometry
 */

namespace bisky::scene
{

class Scene
{
  public:
    explicit Scene(gfx::Window *const window, gfx::Device *const device, std::string_view name);
    ~Scene();

    Scene(const Scene &)                    = delete;
    const Scene &operator=(const Scene &)   = delete;
    Scene(const Scene &&)                   = delete;
    const Scene &&operator=(const Scene &&) = delete;

  public: // Public functions
    void update(const core::GameTimer *const timer);
    void updateSceneBuffer();

  public: // Getter functions
    const std::vector<std::shared_ptr<RenderObject>> &getRenderObjects() const;
    Camera *const                                     getCamera() const;
    const std::vector<PointLight>                    &getLights() const;

  private: // Private functions
    void initDefaultScene();

  private:
    gfx::Window *const                         m_window;
    gfx::Device *const                         m_device;
    std::string_view                           m_name;
    std::vector<std::shared_ptr<RenderObject>> m_renderObjects;
    std::unique_ptr<Camera>                    m_camera; // every scene has a camera - later hold more cameras
    std::unique_ptr<gfx::SceneBuffer>          m_sceneBuffer;
    std::vector<PointLight>                    m_lights;
};

} // namespace bisky::scene