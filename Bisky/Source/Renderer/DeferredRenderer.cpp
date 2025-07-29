#include "Common.hpp"

#include "Graphics/Device.hpp"
#include "Graphics/FrameResource.hpp"
#include "Graphics/Material.hpp"
#include "Graphics/Window.hpp"
#include "Renderer/DeferredRenderer.hpp"
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

DeferredRenderer::~DeferredRenderer()
{
}

auto DeferredRenderer::geometryPass(scene::Scene *const scene, gfx::FrameResource *const frameResource) const -> void
{
    auto *cmdList = frameResource->graphicsCommandList.get();

    const std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargetViews = {
        m_gBuffer.position->rtvDescriptor.cpu,
        m_gBuffer.normal->rtvDescriptor.cpu,
        m_gBuffer.albedo->rtvDescriptor.cpu,
    };

    float color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    cmdList->clearRenderTargetViews(renderTargetViews, color);
    cmdList->clearDepthStencilView(m_device->getDepthStencilView(), 1.0f, 0u);

    cmdList->setRenderTargets(renderTargetViews, m_device->getDepthStencilView());

    cmdList->setRootSignature(m_graphicsPipeline->getRootSignature());
    cmdList->setPipelineState(m_graphicsPipeline->getPipelineState());

    cmdList->setConstantBufferView(
        m_graphicsPipeline->getRootParameter("sceneBuffer"),
        frameResource->sceneBuffer->resource->GetGPUVirtualAddress()
    );

    auto renderObjects = scene->getRenderObjects();
    for (const size_t i : std::views::iota(0u, renderObjects.size()))
    {
        auto *ro = renderObjects[i].get();

        auto objectAlloc  = frameResource->resourceAllocator->allocate(sizeof(gfx::ObjectBuffer));
        auto objectBuffer = reinterpret_cast<gfx::ObjectBuffer *>(objectAlloc.cpuBase);
        XMStoreFloat4x4(&objectBuffer->world, ro->transform->getLocalToWorld());
        XMStoreFloat4x4(&objectBuffer->inverseWorld, XMMatrixInverse(nullptr, XMLoadFloat4x4(&objectBuffer->world)));
        XMStoreFloat4x4(
            &objectBuffer->transposeInverseWorld, XMMatrixTranspose(XMLoadFloat4x4(&objectBuffer->inverseWorld))
        );
        cmdList->setConstantBufferView(m_graphicsPipeline->getRootParameter("objectBuffer"), objectAlloc.gpuBase);

        DeferredRenderer::RenderResource renderResource{};
        renderResource.vertexBufferIndex = gfx::Buffer::GetSrvIndex(ro->mesh->vertexBuffer.get());

        for (const auto &submesh : ro->mesh->submeshes)
        {
            renderResource.diffuseMapIndex = gfx::Texture::GetSrvIndex(submesh.material->diffuseTexture.get());
            cmdList->set32BitConstants(m_graphicsPipeline->getRootParameter("renderResource"), 2u, &renderResource);

            cmdList->setIndexBuffer({
                .bufferLocation = ro->mesh->indexBuffer->resource->GetGPUVirtualAddress(),
                .sizeInBytes    = ro->mesh->indexBufferByteSize,
                .format         = ro->mesh->indexFormat,
            });
            cmdList->setPrimitiveTopology(ro->primitiveTopology);

            cmdList->drawIndexedInstanced(submesh);
        }
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