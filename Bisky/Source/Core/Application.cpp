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
    m_renderer        = std::make_unique<renderer::ForwardRenderer>(m_window.get(), m_backend.get());
    m_finalRenderPass = std::make_unique<renderer::FinalRenderPass>(m_backend.get());
    m_scene           = std::make_unique<scene::Scene>(m_window.get(), m_backend.get(), "test");

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
            m_renderer->resize(m_window->getWidth(), m_window->getHeight());
            m_scene->getCamera()->setLens(m_window->getAspectRatio(), 0.1f, 100.0f);
            m_scene->updateSceneBuffer();
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

        // -------------- get next swapchain buffer index --------------
        m_backend->update();

        //// -------------- transition resource to render target --------------
        // cmdList->addBarrier(
        //     m_backend->getRenderTargetBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET
        //);
        // cmdList->dispatchBarriers();

        //// -------------- clear render target view --------------
        // float color[4] = {0.15f, 0.15f, 0.15f, 1.0f};
        // cmdList->clearRenderTargetView(m_backend->getRenderTargetView(), color);
        // cmdList->clearDepthStencilView(m_backend->getDepthStencilView(), 1.0f, 0);

        //// -------------- set viewport and scissor --------------
        // cmdList->setViewport(m_backend->getViewport());
        // cmdList->setScissorRect(m_backend->getScissor());

        //// -------------- set render targets --------------
        // cmdList->setRenderTargets(m_backend->getRenderTargetView(), m_backend->getDepthStencilView());

        // -------------- draw with our renderer --------------
        m_renderer->draw(renderer::RenderLayer::Opaque, frameResource, m_scene.get(), m_frameStats.get());

        // -------------- draw to final render pass --------------
        m_finalRenderPass->draw(frameResource, m_renderer->getRenderTexture(m_backend->getCurrentFrameResourceIndex()));

        // -------------- draw imgui --------------
        m_editor->beginFrame();
        ImGui::Begin("Debug");
        ImGui::Text("Frame Time: %f", m_frameStats->frameTime);
        ImGui::Text("Scene Update Time: %f", m_frameStats->sceneUpdateTime);
        ImGui::Text("Mesh Draw Time: %f", m_frameStats->meshDrawTime);
        ImGui::Text("Triangle Count: %i", m_frameStats->triangleCount);
        ImGui::Text("Draw Count: %i", m_frameStats->drawCount);
        ImGui::End();
        m_editor->render(m_scene.get());
        m_editor->endFrame(cmdList, m_backend.get());

        // -------------- transition resource to present --------------
        cmdList->addBarrier(
            m_backend->getRenderTargetBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT
        );
        cmdList->dispatchBarriers();

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

void Application::OnMouseMove(WPARAM key, int x, int y)
{
}

void Application::OnLeftMouseDown(WPARAM key, int x, int y)
{
}

void Application::OnLeftMouseUp()
{
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