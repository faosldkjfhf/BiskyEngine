#include "Bisky.hpp"

struct Input : public bisky::core::Input
{
    bisky::scene::Scene *scene;
    bisky::gfx::Window  *window;

    virtual void OnMouseMove(WPARAM key, int x, int y) override
    {
        scene->getArcballCamera()->onMove(x, y);
    }

    virtual void OnLeftMouseDown(WPARAM key, int x, int y) override
    {
        scene->getArcballCamera()->onBegin(x, y);
    };

    virtual void OnLeftMouseUp() override
    {
        scene->getArcballCamera()->onEnd();
    };

    virtual void OnKeyDown(WPARAM key) override
    {
        switch (key)
        {
        case VK_ESCAPE:
            window->setShouldClose();
            break;
        case VK_F11:
            window->setFullscreenState(!window->getFullscreenState());
            break;
        default:
            break;
        }
    };
};

int main()
{
    using namespace bisky;
    SET_DEFAULT_WORKING_DIRECTORY();

    // ----- initialize d3d12 backend -----
    Input input;
    auto  debugLayer = std::make_unique<gfx::DebugLayer>();
    auto  window     = std::make_unique<gfx::Window>(&input, 860u, 640u, "Sandbox");
    auto  device     = std::make_unique<gfx::Device>(window.get());

    // ----- initializer renderer and scene -----
    auto geometryRenderPass = std::make_unique<renderer::ForwardRenderer>(window.get(), device.get());
    auto finalRenderPass    = std::make_unique<renderer::FinalRenderPass>(device.get());
    auto skyboxRenderPass   = std::make_unique<renderer::SkyboxRenderPass>(device.get());
    auto scene              = std::make_unique<scene::Scene>(window.get(), device.get(), "test scene");

    // ----- initialize input -----
    input.window = window.get();
    input.scene  = scene.get();

    // ----- initialize editor -----
    auto editor     = std::make_unique<editor::Editor>(window.get(), device.get());
    auto frameStats = std::make_unique<core::FrameStats>();

    // ----- run loop -----
    window->setFullscreenState(TRUE);
    while (!window->shouldClose())
    {
        // ----- begin timer -----
        auto start = std::chrono::steady_clock::now();

        // ----- update window -----
        window->update();
        if (window->shouldResize())
        {
            device->getDirectCommandQueue()->flush();
            window->resize(device.get());
            scene->getArcballCamera()->resize(window->getWidth(), window->getHeight());
        }

        // ----- update scene -----
        auto sceneStart = std::chrono::steady_clock::now();
        scene->update(nullptr);
        auto sceneEnd               = std::chrono::steady_clock::now();
        auto sceneElapsed           = std::chrono::duration_cast<std::chrono::microseconds>(sceneEnd - sceneStart);
        frameStats->sceneUpdateTime = sceneElapsed.count() / 1000.0f;

        // ----- wait for previous commands to finish -----
        device->incrementFrameResourceIndex();
        auto *frame = device->getFrameResource();
        device->getDirectCommandQueue()->waitForFence(frame->fenceValue);
        frame->resourceAllocator->reset();

        // ----- reset command list -----
        auto *cmdList = frame->graphicsCommandList.get();
        cmdList->reset();

        // ----- begin frame -----
        device->beginFrame(cmdList);

        // ----- draw geometry -----
        geometryRenderPass->draw(renderer::RenderLayer::Opaque, frame, scene.get(), frameStats.get());

        // ----- draw skybox -----
        skyboxRenderPass->draw(frame, scene.get(), frameStats.get());

        // ----- copy HDR render target to swapchain buffer image -----
        finalRenderPass->draw(frame, frameStats.get());

        // ----- draw editor UI -----
        editor->beginFrame();
        editor->draw(scene.get());
        editor->draw(frameStats.get());
        editor->endFrame(cmdList, device.get());

        // ----- end frame -----
        device->endFrame(cmdList);

        // ----- execute command list -----
        std::array<const gfx::CommandList *, 1> cmdLists = {cmdList};
        device->getDirectCommandQueue()->executeCommandLists(cmdLists);

        // ----- present swapchain image -----
        device->getSwapChain()->Present(1u, 0u);

        // ----- signal fence completed -----
        frame->fenceValue = device->getDirectCommandQueue()->signal();

        // ----- calculate frame time -----
        auto end              = std::chrono::steady_clock::now();
        auto elapsed          = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        frameStats->frameTime = elapsed.count() / 1000.0f;
    }

    // ----- exit -----
    device->getDirectCommandQueue()->flush();
    core::ResourceManager::get().reset();
    LOG_INFO("Exiting");
    return 0;
}