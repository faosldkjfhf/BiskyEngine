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

// -- TODO: Eventually I want to multithread the engine
// -- render/draw stuff
void RenderThread()
{
}

// -- run compute shaders
void ComputeThread()
{
}

// -- run copy operations
void CopyThread()
{
}

int main()
{
    using namespace bisky;
    SET_DEFAULT_WORKING_DIRECTORY();

    // -- initialize d3d12 backend
    Input input;
    auto  debugLayer = std::make_unique<gfx::DebugLayer>();
    auto  window     = std::make_unique<gfx::Window>(&input, 1280u, 960u, "Sandbox");
    auto  device     = std::make_unique<gfx::Device>(window.get());

    // -- initializer renderer and scene
    auto lightDebugRenderPass = std::make_unique<renderer::LightDebugRenderPass>(device.get());
    auto skyboxRenderPass     = std::make_unique<renderer::SkyboxRenderPass>(device.get());
    // auto bloomComputePass     = std::make_unique<renderer::BloomComputePass>(device.get());
    auto finalRenderPass = std::make_unique<renderer::FinalRenderPass>(device.get());
    auto scene           = std::make_unique<scene::Scene>(window.get(), device.get(), "test scene");

    // -- initialize deferred renderer
    auto deferredRenderer = std::make_unique<renderer::DeferredRenderer>(device.get(), window.get());

    // -- initialize skybox compute pipeline
    auto computeSkybox = std::make_unique<gfx::ComputePipeline>(
        device.get(),
        gfx::ComputePipelineStateDesc{
            .computeShader = {.name = "Compute\\Skybox.hlsl", .entryPoint = L"CsMain"},
        }
    );

    // -- initialize bloom pipeline
    auto computeBloom = std::make_unique<gfx::ComputePipeline>(
        device.get(),
        gfx::ComputePipelineStateDesc{
            .computeShader = {.name = "Compute\\Bloom.hlsl", .entryPoint = L"CsMain"},
        }
    );

    // -- initialize input
    input.window = window.get();
    input.scene  = scene.get();

    // -- initialize editor
    auto editor     = std::make_unique<editor::Editor>(window.get(), device.get());
    auto frameStats = std::make_unique<core::FrameStats>();

    // -- run loop
    while (!window->shouldClose())
    {
        // -- begin timer
        auto start = std::chrono::steady_clock::now();

        // -- update window
        window->update();
        if (window->shouldResize())
        {
            device->getDirectCommandQueue()->flush();
            window->resize(device.get());
            deferredRenderer->resize(window.get());
            scene->getArcballCamera()->resize(window->getWidth(), window->getHeight());
        }

        // -- reset draw statistics
        frameStats->drawCount     = 0u;
        frameStats->triangleCount = 0u;

        // -- update scene
        auto sceneStart = std::chrono::steady_clock::now();
        scene->update(nullptr);
        auto sceneEnd               = std::chrono::steady_clock::now();
        auto sceneElapsed           = std::chrono::duration_cast<std::chrono::microseconds>(sceneEnd - sceneStart);
        frameStats->sceneUpdateTime = sceneElapsed.count() / 1000.0f;

        // -- wait for previous commands to finish
        auto *frame = device->nextFrameResource();

        // -- fill scene buffer
        // FIXME: there's a better way to do this probably
        auto             camera = scene->getArcballCamera();
        gfx::SceneBuffer buffer{};
        XMStoreFloat4x4(&buffer.view, camera->getView());
        XMStoreFloat4x4(&buffer.projection, camera->getProjection());
        XMStoreFloat4x4(&buffer.viewProjection, camera->getView() * scene->getArcballCamera()->getProjection());
        XMStoreFloat4(&buffer.viewPosition, camera->getPosition());

        void *sceneBuffer;
        frame->sceneBuffer->resource->Map(0, nullptr, &sceneBuffer);
        memcpy(sceneBuffer, &buffer, (sizeof(buffer) + 255) & ~255);
        frame->sceneBuffer->resource->Unmap(0, nullptr);

        // -- reset command list
        auto *cmdList = frame->graphicsCommandList.get();
        cmdList->reset();

        // -- begin frame
        device->beginFrame(cmdList);

        // -- clear HDR render target
        float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        cmdList->clearRenderTargetView(device->getHdrRenderTargetView(), clearColor);

        // -- transition HDR render target to unordered access
        // cmdList->addBarrier(
        //    device->getHdrRenderTargetBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET,
        //    D3D12_RESOURCE_STATE_UNORDERED_ACCESS
        //);
        // cmdList->dispatchBarriers();

        // -- set viewport and scissor
        cmdList->setViewport(device->getViewport());
        cmdList->setScissorRect(device->getScissor());

        // -- set descriptor heaps
        std::array<const gfx::DescriptorHeap *const, 1> heaps = {device->getCbvSrvUavHeap()};
        cmdList->setDescriptorHeaps(heaps);

        // -- draw skybox with compute pipeline
        // cmdList->setPipelineState(computeSkybox->getPipelineState());
        // cmdList->setComputeRootSignature(computeSkybox->getRootSignature());

        // -- bind render resource
        // struct
        //{
        //    int32_t outputImageIndex;
        //} renderResource                = {};
        // renderResource.outputImageIndex = gfx::Texture::GetUavIndex(device->getHdrRenderTargetBuffer());
        // cmdList->setCompute32BitConstants(computeSkybox->getRootParameter("renderResource"), renderResource);

        //// -- dispatch threads
        // cmdList->dispatch(static_cast<UINT>(window->getWidth() / 8u), static_cast<UINT>(window->getHeight() /
        // 8u), 1u);

        //// -- transition back to render target
        // cmdList->addBarrier(
        //     device->getHdrRenderTargetBuffer(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        //     D3D12_RESOURCE_STATE_RENDER_TARGET
        //);
        // cmdList->dispatchBarriers();

        // -- deferred rendering pass
        deferredRenderer->geometryPass(scene.get(), frame, frameStats.get());
        deferredRenderer->lightPass(scene.get(), frame, frameStats.get());

        // -- draw light debug
        // lightDebugRenderPass->draw(frame, scene.get(), frameStats.get());

        // -- draw skybox
        skyboxRenderPass->draw(frame, scene.get(), frameStats.get());

        // -- bind bloom pipeline
        cmdList->setPipelineState(computeBloom->getPipelineState());
        cmdList->setComputeRootSignature(computeBloom->getRootSignature());

        // -- bind render resource
        struct
        {
            int outputImageIndex;
            int horizontal;
        } renderResource = {
            .outputImageIndex = gfx::Texture::GetUavIndex(deferredRenderer->getBloomTexture()),
            .horizontal       = 1u,
        };

        // -- blur 5 times horizontally and 5 times vertically
        bool horizontal = true;
        for (UINT i = 0; i < 10; i++)
        {
            // -- bind renderResource
            renderResource.horizontal = static_cast<int>(horizontal);
            cmdList->setCompute32BitConstants(computeBloom->getRootParameter("renderResource"), renderResource);

            // -- dispatch compute shader
            cmdList->dispatch(
                static_cast<UINT>(window->getWidth() / 8u), static_cast<UINT>(window->getHeight() / 8u), 1u
            );

            horizontal = !horizontal;
        }

        // -- transition bloom texture to common
        cmdList->addBarrier(
            deferredRenderer->getBloomTexture(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON
        );
        cmdList->dispatchBarriers();

        // -- copy HDR render target to swapchain buffer image
        finalRenderPass->draw(
            device->getHdrRenderTargetBuffer(), deferredRenderer->getBloomTexture(), frame, frameStats.get()
        );

        // -- draw editor UI
        editor->beginFrame();
        editor->draw(frameStats.get());
        editor->endFrame(cmdList, device.get());

        // -- end frame
        device->endFrame(cmdList);

        // -- execute command list
        std::array<const gfx::CommandList *, 1> cmdLists = {cmdList};
        device->getDirectCommandQueue()->executeCommandLists(cmdLists);

        // -- present swapchain image
        device->present(1u);

        // -- signal fence completed
        frame->fenceValue = device->getDirectCommandQueue()->signal();

        // -- calculate frame time
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