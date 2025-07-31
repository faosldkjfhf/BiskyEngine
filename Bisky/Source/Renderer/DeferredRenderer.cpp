#include "Common.hpp"

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
    // ----- position texture -----
    m_gBuffer.position = device->createTexture2D(
        window->getWidth(), window->getHeight(), DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
    );
    device->createRenderTargetView(m_gBuffer.position.get());
    device->createShaderResourceView(m_gBuffer.position.get());

    // ----- normal texture -----
    m_gBuffer.normal = device->createTexture2D(
        window->getWidth(), window->getHeight(), DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
    );
    device->createRenderTargetView(m_gBuffer.normal.get());
    device->createShaderResourceView(m_gBuffer.normal.get());

    // ----- albedo texture -----
    m_gBuffer.albedo = device->createTexture2D(
        window->getWidth(), window->getHeight(), DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
    );
    device->createRenderTargetView(m_gBuffer.albedo.get());
    device->createShaderResourceView(m_gBuffer.albedo.get());

    // ----- create graphics pipeline -----
    std::array<DXGI_FORMAT, 3> formats = {
        m_gBuffer.position->resource->GetDesc().Format,
        m_gBuffer.normal->resource->GetDesc().Format,
        m_gBuffer.albedo->resource->GetDesc().Format,
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
        std::array<DXGI_FORMAT, 1> renderFormats = {m_device->getHdrRenderTargetFormat()};

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

    // ----- screen quad render object -----
    m_quad       = std::make_unique<scene::RenderObject>();
    m_quad->mesh = core::ResourceManager::get().getMesh("ScreenQuad");
}

DeferredRenderer::~DeferredRenderer()
{
}

auto DeferredRenderer::geometryPass(scene::Scene *const scene, gfx::FrameResource *const frameResource) const -> void
{
    auto *cmdList = frameResource->graphicsCommandList.get();

    cmdList->addBarrier(m_gBuffer.position.get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
    cmdList->addBarrier(m_gBuffer.normal.get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
    cmdList->addBarrier(m_gBuffer.albedo.get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
    cmdList->dispatchBarriers();

    const std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargetViews = {
        m_gBuffer.position->rtvDescriptor.cpu,
        m_gBuffer.normal->rtvDescriptor.cpu,
        m_gBuffer.albedo->rtvDescriptor.cpu,
    };

    // ----- clear render targets -----
    float color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    cmdList->clearRenderTargetViews(renderTargetViews, color);
    cmdList->clearDepthStencilView(m_device->getDepthStencilView(), 1.0f, 0u);

    // ----- set viewport and scissor -----
    cmdList->setViewport(m_device->getViewport());
    cmdList->setScissorRect(m_device->getScissor());

    // ----- set render targets -----
    cmdList->setRenderTargets(renderTargetViews, m_device->getDepthStencilView());

    // ----- bind pipeline state -----
    cmdList->setPipelineState(m_graphicsPipeline->getPipelineState());

    // ----- set descriptor heaps -----
    std::array<const gfx::DescriptorHeap *const, 1> heaps = {m_device->getCbvSrvUavHeap()};
    cmdList->setDescriptorHeaps(heaps);

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
        XMStoreFloat4x4(&objectBuffer->world, ro->transform->getLocalToWorld());
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
            cmdList->set32BitConstants(m_graphicsPipeline->getRootParameter("renderResource"), 2u, &renderResource);

            // ----- bind index buffer -----
            cmdList->setIndexBuffer({
                .bufferLocation = ro->mesh->indexBuffer->resource->GetGPUVirtualAddress(),
                .sizeInBytes    = ro->mesh->indexBufferByteSize,
                .format         = ro->mesh->indexFormat,
            });
            cmdList->setPrimitiveTopology(ro->primitiveTopology);

            // ----- draw submesh -----
            cmdList->drawIndexedInstanced(submesh);
        }
    }

    // ----- set gbuffer as render target -----
    cmdList->addBarrier(m_gBuffer.position.get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
    cmdList->addBarrier(m_gBuffer.normal.get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
    cmdList->addBarrier(m_gBuffer.albedo.get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
    cmdList->dispatchBarriers();
}

auto DeferredRenderer::lightPass(scene::Scene *const scene, gfx::FrameResource *const frameResource) const -> void
{
    auto *cmdList = frameResource->graphicsCommandList.get();

    // ----- clear HDR render target -----
    float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    cmdList->clearRenderTargetView(m_device->getHdrRenderTargetView(), clearColor);

    // ----- set render target view -----
    cmdList->setRenderTargets(m_device->getHdrRenderTargetView());

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
    renderResource.positionMapIndex  = gfx::Texture::GetSrvIndex(m_gBuffer.position.get());
    renderResource.normalMapIndex    = gfx::Texture::GetSrvIndex(m_gBuffer.normal.get());
    renderResource.albedoMapIndex    = gfx::Texture::GetSrvIndex(m_gBuffer.albedo.get());
    cmdList->set32BitConstants(m_lightPassPipeline->getRootParameter("renderResource"), 4u, &renderResource);

    // ----- draw submesh -----
    for (const auto &submesh : m_quad->mesh->submeshes)
    {
        cmdList->drawIndexedInstanced(submesh);
    }
}

auto DeferredRenderer::resize(gfx::Window *const window) -> void
{
    m_gBuffer.position.reset();
    m_gBuffer.normal.reset();
    m_gBuffer.albedo.reset();

    // ----- position texture -----
    m_gBuffer.position = m_device->createTexture2D(
        window->getWidth(), window->getHeight(), DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
    );
    m_device->createRenderTargetView(m_gBuffer.position.get());
    m_device->createShaderResourceView(m_gBuffer.position.get());

    // ----- normal texture -----
    m_gBuffer.normal = m_device->createTexture2D(
        window->getWidth(), window->getHeight(), DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
    );
    m_device->createRenderTargetView(m_gBuffer.normal.get());
    m_device->createShaderResourceView(m_gBuffer.normal.get());

    // ----- albedo texture -----
    m_gBuffer.albedo = m_device->createTexture2D(
        window->getWidth(), window->getHeight(), DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
    );
    m_device->createRenderTargetView(m_gBuffer.albedo.get());
    m_device->createShaderResourceView(m_gBuffer.albedo.get());
}

auto DeferredRenderer::getGBuffer() -> GBuffer *
{
    return &m_gBuffer;
}

} // namespace bisky::renderer