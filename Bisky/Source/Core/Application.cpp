#include "Common.hpp"

#include "Core/Application.hpp"
#include "Core/ResourceManager.hpp"

namespace bisky::core
{

Application::Application(uint32_t width, uint32_t height, const std::string &title)
{
    // -------------- initialize core d3d12 objects --------------
    m_debug   = std::make_unique<gfx::DebugLayer>();
    m_window  = std::make_unique<gfx::Window>(this, width, height, title);
    m_backend = std::make_unique<gfx::Device>(m_window.get());

    // -------------- initialize the timer --------------
    m_timer = std::make_unique<core::GameTimer>();

    // -------------- initialize our renderer and scene --------------
    m_renderer         = std::make_unique<renderer::ForwardRenderer>(m_window.get(), m_backend.get());
    m_finalRenderPass  = std::make_unique<renderer::FinalRenderPass>(m_backend.get());
    m_skyboxRenderPass = std::make_unique<renderer::SkyboxRenderPass>(m_backend.get());
    m_scene            = std::make_unique<scene::Scene>(m_window.get(), m_backend.get(), "test");

    // -------------- initialize the editor --------------
    m_editor     = std::make_unique<editor::Editor>(m_window.get(), m_backend.get());
    m_frameStats = std::make_unique<core::FrameStats>();
}

Application::~Application()
{
    ResourceManager::get().reset();
    m_timer.reset();
    m_editor.reset();
    m_scene.reset();
    m_finalRenderPass.reset();
    m_renderer.reset();
    m_backend.reset();
    m_window.reset();
    m_debug.reset();
}

void Application::run()
{
    m_timer->reset();
    m_window->setFullscreenState(true);
    while (!m_window->shouldClose())
    {
        auto start = std::chrono::system_clock::now();

        // -------------- wait for the commands to catch up --------------
        m_backend->incrementFrameResourceIndex();
        auto *frameResource = m_backend->getFrameResource();
        auto *cmdList       = frameResource->graphicsCommandList.get();
        m_backend->getDirectCommandQueue()->waitForFence(frameResource->fenceValue);

        // -------------- parse window inputs --------------
        m_window->update();
        if (m_window->shouldResize())
        {
            m_backend->getDirectCommandQueue()->flush();
            m_window->resize(m_backend.get());
            m_scene->getArcball()->setLens(m_window->getAspectRatio(), 0.1f, 100.0f);
        }

        // -------------- reset the command list --------------
        cmdList->reset();
        frameResource->resourceAllocator->reset();

        {
            auto start = std::chrono::system_clock::now();

            // -------------- update scene --------------
            m_scene->update(m_timer.get());

            auto end                      = std::chrono::system_clock::now();
            auto elapsed                  = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            m_frameStats->sceneUpdateTime = elapsed.count() / 1000.0f;
        }

        // -------------- get next swapchain buffer index and transition render targets --------------
        m_backend->beginFrame(cmdList);

        // -------------- draw with our renderer --------------
        m_renderer->draw(renderer::RenderLayer::Opaque, frameResource, m_scene.get(), m_frameStats.get());

        // -------------- draw skybox --------------
        m_skyboxRenderPass->draw(frameResource, m_scene.get(), m_frameStats.get());

        // -------------- transition the HDR render target to common state --------------
        m_backend->endHdrFrame(cmdList);

        // -------------- draw final render pass --------------
        m_finalRenderPass->draw(frameResource, m_frameStats.get());

        // -------------- draw imgui --------------
        m_editor->beginFrame();
        ImGui::Begin("Debug");
        ImGui::Text("Triangle Count: %i", m_frameStats->triangleCount);
        ImGui::Text("Draw Count: %i", m_frameStats->drawCount);
        ImGui::Text("Frame Time: %f", m_frameStats->frameTime);
        ImGui::Text("Scene Update Time: %f", m_frameStats->sceneUpdateTime);
        ImGui::Text("Mesh Draw Time: %f", m_frameStats->meshDrawTime);
        ImGui::Text("Final Render Draw Time: %f", m_frameStats->finalRenderDrawTime);
        ImGui::End();
        m_editor->render(m_scene.get());
        m_editor->endFrame(cmdList, m_backend.get());

        // -------------- transition resource to present --------------
        m_backend->endFrame(cmdList);

        // -------------- execute command list --------------
        std::array<const bisky::gfx::CommandList *const, 1> commandLists = {cmdList};
        m_backend->getDirectCommandQueue()->executeCommandLists(commandLists);

        // -------------- present --------------
        m_backend->getSwapChain()->Present(0, 0);

        // -------------- signal next fence --------------
        frameResource->fenceValue = m_backend->getDirectCommandQueue()->signal();

        // -------------- tick timer --------------
        m_timer->tick();

        // -------------- update frame time --------------
        auto end                = std::chrono::system_clock::now();
        auto elapsed            = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        m_frameStats->frameTime = elapsed.count() / 1000.0f;
    }

    // -------------- flush command queue before exiting --------------
    m_backend->getDirectCommandQueue()->flush();
    LOG_INFO("Exiting");
}

inline static dx::XMFLOAT2 convertToNdc(gfx::Window *const window, int x, int y)
{
    dx::XMFLOAT2 ndc{};

    // map between [0, 1]
    ndc.x = static_cast<float>(x) / window->getWidth();
    ndc.y = 1.0f - (static_cast<float>(y) / window->getHeight());

    // map to ndc [-1, 1]
    ndc.x = std::clamp(ndc.x * 2.0f - 1.0f, -1.0f, 1.0f);
    ndc.y = std::clamp(ndc.y * 2.0f - 1.0f, -1.0f, 1.0f);

    return ndc;
}

void Application::OnMouseMove(WPARAM key, int x, int y)
{
    if (m_lmbDown)
    {
        auto *arcball = m_scene->getArcball();

        auto end   = convertToNdc(m_window.get(), x, y);
        auto start = convertToNdc(m_window.get(), m_lastMousePosition.x, m_lastMousePosition.y);
        arcball->rotate(start, end);

        m_lastMousePosition = {.x = x, .y = y};
    }
}

void Application::OnLeftMouseDown(WPARAM key, int x, int y)
{
    m_lmbDown           = true;
    m_lastMousePosition = {.x = x, .y = y};
}

void Application::OnLeftMouseUp()
{
    m_lmbDown = false;
}

void Application::OnKeyDown(WPARAM key)
{
    switch (key)
    {
    case VK_ESCAPE:
        m_window->setShouldClose();
        break;
    case VK_F11:
        m_window->setFullscreenState(!m_window->getFullscreenState());
        break;
    default:
        break;
    }
}

} // namespace bisky::core