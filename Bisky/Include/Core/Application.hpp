#pragma once

#include "Core/GameTimer.hpp"
#include "Core/ResourceManager.hpp"
#include "Editor/Editor.hpp"
#include "Graphics/DebugLayer.hpp"
#include "Graphics/Device.hpp"
#include "Graphics/FrameResource.hpp"
#include "Graphics/Window.hpp"
#include "Renderer/ForwardRenderer.hpp"
#include "Scene/Scene.hpp"

namespace bisky::core
{

/*
 * A basic application class.
 *
 * TODO: Make it extendable and overrideable.
 */
class Application
{
  public:
    /*
     * Initializes all core and graphics related objects.
     *
     * @param width The width of the window to use.
     * @param height The height of the window to use.
     * @param title The title of the window to use.
     */
    explicit Application(uint32_t width, uint32_t height, const std::string &title);

    /*
     * Resets all stored objects.
     */
    ~Application();

    /*
     * Runs the application.
     * This can be overriden.
     */
    virtual void run();

    Application(const Application &)                    = delete;
    const Application &operator=(const Application &)   = delete;
    Application(const Application &&)                   = delete;
    const Application &&operator=(const Application &&) = delete;

  protected:
    void calculateFrameStats();

    std::unique_ptr<gfx::DebugLayer> m_debug;
    std::unique_ptr<gfx::Window>     m_window;
    std::unique_ptr<gfx::Device>     m_backend;

    std::unique_ptr<renderer::ForwardRenderer> m_renderer;
    std::unique_ptr<scene::Scene>              m_scene;

    std::unique_ptr<editor::Editor> m_editor;

    std::unique_ptr<core::GameTimer> m_timer;
    float                            m_fps  = 0.0f;
    float                            m_mspf = 0.0f;
};

} // namespace bisky::core