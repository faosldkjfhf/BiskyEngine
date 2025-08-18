#include "Common.hpp"

#include "Core/FrameStats.hpp"
#include "Core/ResourceManager.hpp"
#include "Graphics/Device.hpp"
#include "Graphics/FrameResource.hpp"
#include "Graphics/Material.hpp"
#include "Graphics/Window.hpp"
#include "Renderer/DeferredRenderer.hpp"
#include "Scene/RenderObject.hpp"
#include "Scene/Scene.hpp"

namespace bisky::renderer
{

DeferredRenderer::DeferredRenderer(gfx::Device *const device, gfx::Window *const window) : m_device(device)
{
    // -- create geometry buffer
    initGBuffer(window);

    // -- create graphics pipeline
    std::array<DXGI_FORMAT, 4> formats = {
        m_gBuffer.positionAmbientOcclusion->resource->GetDesc().Format,
        m_gBuffer.normalRoughness->resource->GetDesc().Format,
        m_gBuffer.albedoMetallic->resource->GetDesc().Format,
        m_gBuffer.emissive->resource->GetDesc().Format,
    };

    {
        gfx::GraphicsPipelineStateDesc desc = {
            .vertexShader = {.name = "Geometry\\Deferred.hlsl", .entryPoint = L"VsMain"},
            .pixelShader  = {.name = "Geometry\\Deferred.hlsl", .entryPoint = L"PsMain"},
            .rtvCount     = static_cast<UINT32>(formats.size()),
            .rtvFormats   = formats,
            .dsvFormat    = m_device->getDepthStencilFormat(),
            .cullMode     = gfx::CullMode::Back,
            .frontFace    = gfx::FrontFace::Clockwise,
        };

        m_graphicsPipeline = std::make_unique<gfx::GraphicsPipeline>(m_device, desc);
    }

    {
        std::array<DXGI_FORMAT, 2> renderFormats = {
            m_device->getHdrRenderTargetFormat(), DXGI_FORMAT_R16G16B16A16_FLOAT
        };

        gfx::GraphicsPipelineStateDesc desc = {
            .vertexShader = {.name = "Lighting\\LightPass.hlsl", .entryPoint = L"VsMain"},
            .pixelShader  = {.name = "Lighting\\LightPass.hlsl", .entryPoint = L"PsMain"},
            .rtvCount     = static_cast<UINT32>(renderFormats.size()),
            .rtvFormats   = renderFormats,
            .cullMode     = gfx::CullMode::Back,
            .frontFace    = gfx::FrontFace::Clockwise,
        };

        m_lightPassPipeline = std::make_unique<gfx::GraphicsPipeline>(m_device, desc);
    }

    // -- screen quad render object
    m_quad       = std::make_unique<scene::RenderObject>();
    m_quad->mesh = core::ResourceManager::get().getMesh("ScreenQuad");
}

DeferredRenderer::~DeferredRenderer()
{
}

auto DeferredRenderer::geometryPass(
    scene::Scene *const scene, gfx::FrameResource *const frameResource, core::FrameStats *const frameStats
) const -> void
{
    auto *cmdList = frameResource->graphicsCommandList.get();

    const std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargetViews = {
        m_gBuffer.positionAmbientOcclusion->rtvDescriptor.cpu,
        m_gBuffer.normalRoughness->rtvDescriptor.cpu,
        m_gBuffer.albedoMetallic->rtvDescriptor.cpu,
        m_gBuffer.emissive->rtvDescriptor.cpu,
    };

    // ----- transition gbuffer to render target -----
    cmdList->addBarrier(
        m_gBuffer.positionAmbientOcclusion.get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    cmdList->addBarrier(
        m_gBuffer.normalRoughness.get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    cmdList->addBarrier(
        m_gBuffer.albedoMetallic.get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    cmdList->addBarrier(m_gBuffer.emissive.get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
    cmdList->dispatchBarriers();

    // ----- clear render targets -----
    float color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    cmdList->clearRenderTargetViews(renderTargetViews, color);
    cmdList->clearDepthStencilView(m_device->getDepthStencilView(), 1.0f, 0u);

    // ----- set render targets -----
    cmdList->setRenderTargets(renderTargetViews, m_device->getDepthStencilView());

    // ----- bind pipeline state -----
    cmdList->setPipelineState(m_graphicsPipeline->getPipelineState());

    // ----- bind root signature -----
    cmdList->setRootSignature(m_graphicsPipeline->getRootSignature());

    // ----- bind scene buffer -----
    cmdList->setConstantBufferView(
        m_graphicsPipeline->getRootParameter("sceneBuffer"),
        frameResource->sceneBuffer->resource->GetGPUVirtualAddress()
    );

    // ----- draw scene objects -----
    auto const &renderObjects = scene->getRenderObjects();
    for (const size_t i : std::views::iota(0u, renderObjects.size()))
    {
        auto *ro = renderObjects[i].get();

        // ----- bind object buffer -----
        auto objectAlloc  = frameResource->resourceAllocator->allocate(sizeof(gfx::ObjectBuffer));
        auto objectBuffer = reinterpret_cast<gfx::ObjectBuffer *>(objectAlloc.cpuBase);
        XMStoreFloat4x4(&objectBuffer->world, ro->transform.getLocalToWorld());
        XMStoreFloat4x4(&objectBuffer->inverseWorld, XMMatrixInverse(nullptr, XMLoadFloat4x4(&objectBuffer->world)));
        XMStoreFloat4x4(
            &objectBuffer->transposeInverseWorld, XMMatrixTranspose(XMLoadFloat4x4(&objectBuffer->inverseWorld))
        );
        cmdList->setConstantBufferView(m_graphicsPipeline->getRootParameter("objectBuffer"), objectAlloc.gpuBase);

        // ----- render resource -----
        DeferredRenderer::RenderResource renderResource{};
        renderResource.vertexBufferIndex = gfx::Buffer::GetSrvIndex(ro->mesh->vertexBuffer.get());

        // ----- draw submeshes -----
        for (const auto &submesh : ro->mesh->submeshes)
        {
            // ----- bind render resource -----
            renderResource.diffuseMapIndex = gfx::Texture::GetSrvIndex(submesh.material->diffuseTexture.get());
            renderResource.metallicRoughnessMapIndex =
                gfx::Texture::GetSrvIndex(submesh.material->metallicRoughnessTexture.get());
            renderResource.ambientOcclusionMapIndex =
                gfx::Texture::GetSrvIndex(submesh.material->ambientOccusionTexture.get());
            renderResource.emissiveMapIndex = gfx::Texture::GetSrvIndex(submesh.material->emissiveTexture.get());
            cmdList->set32BitConstants(m_graphicsPipeline->getRootParameter("renderResource"), 5u, &renderResource);

            // ----- bind index buffer -----
            cmdList->setIndexBuffer({
                .bufferLocation = ro->mesh->indexBuffer->resource->GetGPUVirtualAddress(),
                .sizeInBytes    = ro->mesh->indexBufferByteSize,
                .format         = ro->mesh->indexFormat,
            });
            cmdList->setPrimitiveTopology(ro->primitiveTopology);

            // ----- draw submesh -----
            cmdList->drawIndexedInstanced(submesh);

            // ----- update frame stats -----
            frameStats->drawCount++;
            frameStats->triangleCount += submesh.indexCount / 3u;
        }
    }

    // ----- set gbuffer as common -----
    cmdList->addBarrier(
        m_gBuffer.positionAmbientOcclusion.get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON
    );
    cmdList->addBarrier(
        m_gBuffer.normalRoughness.get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON
    );
    cmdList->addBarrier(
        m_gBuffer.albedoMetallic.get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON
    );
    cmdList->addBarrier(m_gBuffer.emissive.get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
    cmdList->dispatchBarriers();
}

auto DeferredRenderer::lightPass(
    scene::Scene *const scene, gfx::FrameResource *const frameResource, core::FrameStats *const frameStats
) const -> void
{
    auto *cmdList = frameResource->graphicsCommandList.get();

    // -- transition to render target
    cmdList->addBarrier(
        frameResource->bloomTexture.get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    cmdList->dispatchBarriers();

    const std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargets = {
        m_device->getHdrRenderTargetView(), frameResource->bloomTexture->rtvDescriptor.cpu
    };

    // ----- set render target view -----
    cmdList->setRenderTargets(renderTargets);

    // ----- set pipeline state and root signature -----
    cmdList->setPipelineState(m_lightPassPipeline->getPipelineState());
    cmdList->setRootSignature(m_lightPassPipeline->getRootSignature());

    // ----- bind scene buffer -----
    cmdList->setConstantBufferView(
        m_lightPassPipeline->getRootParameter("sceneBuffer"),
        frameResource->sceneBuffer->resource->GetGPUVirtualAddress()
    );

    // ----- bind light buffer -----
    auto const &lights           = scene->getLights();
    auto        lightBufferAlloc = frameResource->resourceAllocator->allocate(sizeof(gfx::LightBuffer));
    auto        lightBuffer      = reinterpret_cast<gfx::LightBuffer *>(lightBufferAlloc.cpuBase);
    lightBuffer->numLights       = static_cast<UINT32>(lights.size());
    for (size_t i = 0; i < lights.size(); i++)
    {
        lightBuffer->lights[i] = lights[i];
    }
    cmdList->setConstantBufferView(m_lightPassPipeline->getRootParameter("lightBuffer"), lightBufferAlloc.gpuBase);

    // ----- bind index buffer -----
    cmdList->setIndexBuffer({
        .bufferLocation = m_quad->mesh->indexBuffer->resource->GetGPUVirtualAddress(),
        .sizeInBytes    = m_quad->mesh->indexBufferByteSize,
        .format         = m_quad->mesh->indexFormat,
    });

    // ----- set primitive topology -----
    cmdList->setPrimitiveTopology(m_quad->primitiveTopology);

    // ----- bind render resource -----
    LightPassRenderResource renderResource{};
    renderResource.vertexBufferIndex = gfx::Buffer::GetSrvIndex(m_quad->mesh->vertexBuffer.get());
    renderResource.positionMapIndex  = gfx::Texture::GetSrvIndex(m_gBuffer.positionAmbientOcclusion.get());
    renderResource.normalMapIndex    = gfx::Texture::GetSrvIndex(m_gBuffer.normalRoughness.get());
    renderResource.albedoMapIndex    = gfx::Texture::GetSrvIndex(m_gBuffer.albedoMetallic.get());
    renderResource.emissiveMapIndex  = gfx::Texture::GetSrvIndex(m_gBuffer.emissive.get());
    cmdList->set32BitConstants(m_lightPassPipeline->getRootParameter("renderResource"), 5u, &renderResource);

    // ----- draw submesh -----
    for (const auto &submesh : m_quad->mesh->submeshes)
    {
        cmdList->drawIndexedInstanced(submesh);
    }

    // -- transition to unordered access
    cmdList->addBarrier(
        frameResource->bloomTexture.get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS
    );
    cmdList->dispatchBarriers();
}

auto DeferredRenderer::resize(gfx::Window *const window) -> void
{
    m_gBuffer.positionAmbientOcclusion.reset();
    m_gBuffer.normalRoughness.reset();
    m_gBuffer.albedoMetallic.reset();
    m_gBuffer.emissive.reset();

    initGBuffer(window);
}

auto DeferredRenderer::getGBuffer() -> GBuffer *
{
    return &m_gBuffer;
}

auto DeferredRenderer::initGBuffer(gfx::Window *const window) -> void
{
    // ----- position texture -----
    m_gBuffer.positionAmbientOcclusion = m_device->createTexture2D(
        window->getWidth(), window->getHeight(), DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
    );
    m_device->createRenderTargetView(m_gBuffer.positionAmbientOcclusion.get());
    m_device->createShaderResourceView(m_gBuffer.positionAmbientOcclusion.get());

    // ----- normal texture -----
    m_gBuffer.normalRoughness = m_device->createTexture2D(
        window->getWidth(), window->getHeight(), DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
    );
    m_device->createRenderTargetView(m_gBuffer.normalRoughness.get());
    m_device->createShaderResourceView(m_gBuffer.normalRoughness.get());

    // ----- albedo texture -----
    m_gBuffer.albedoMetallic = m_device->createTexture2D(
        window->getWidth(), window->getHeight(), DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
    );
    m_device->createRenderTargetView(m_gBuffer.albedoMetallic.get());
    m_device->createShaderResourceView(m_gBuffer.albedoMetallic.get());

    // ----- emissive texture -----
    m_gBuffer.emissive = m_device->createTexture2D(
        window->getWidth(), window->getHeight(), DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
    );
    m_device->createRenderTargetView(m_gBuffer.emissive.get());
    m_device->createShaderResourceView(m_gBuffer.emissive.get());
}

} // namespace bisky::renderer