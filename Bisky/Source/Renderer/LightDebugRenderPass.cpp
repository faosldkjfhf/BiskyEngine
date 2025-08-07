#include "Common.hpp"

#include "Core/FrameStats.hpp"
#include "Core/ResourceManager.hpp"
#include "Graphics/Device.hpp"
#include "Graphics/FrameResource.hpp"
#include "Renderer/LightDebugRenderPass.hpp"
#include "Scene/Scene.hpp"

namespace bisky::renderer
{

LightDebugRenderPass::LightDebugRenderPass(gfx::Device *const device) : m_device(device)
{
    initGraphicsPipeline();

    core::ResourceManager::get().loadMesh(device, "Sphere\\sphere.gltf");
    m_sphere       = std::make_unique<scene::RenderObject>();
    m_sphere->mesh = core::ResourceManager::get().getMesh("sphere.000");
    m_sphere->transform->setScale(0.15f, 0.15f, 0.15f);
}

void LightDebugRenderPass::draw(
    gfx::FrameResource *const frameResource, scene::Scene *const scene, core::FrameStats *const frameStats
) const
{
    auto *cmdList = frameResource->graphicsCommandList.get();
    auto  camera  = scene->getArcballCamera();

    // ----- bind depth stencil -----
    cmdList->setRenderTargets(m_device->getHdrRenderTargetView(), m_device->getDepthStencilView());

    // ----- bind pipeline state and root signature -----
    cmdList->setPipelineState(m_graphicsPipeline->getPipelineState());
    cmdList->setRootSignature(m_graphicsPipeline->getRootSignature());

    // ----- bind scene buffer -----
    cmdList->setConstantBufferView(
        m_graphicsPipeline->getRootParameter("sceneBuffer"),
        frameResource->sceneBuffer->resource->GetGPUVirtualAddress()
    );

    // ----- allocate render resource -----
    auto renderResourceAlloc = frameResource->resourceAllocator->allocate(sizeof(LightDebugRenderPass::RenderResource));
    auto renderResource      = reinterpret_cast<LightDebugRenderPass::RenderResource *>(renderResourceAlloc.cpuBase);
    renderResource->vertexBufferIndex = gfx::Buffer::GetSrvIndex(m_sphere->mesh->vertexBuffer.get());

    // ----- draw the lights -----
    for (const size_t i : std::views::iota(0u, scene->getLights().size()))
    {
        // ----- move the light to the correct position -----
        auto light = scene->getLights()[i];
        m_sphere->transform->setTranslation(light.position.x, light.position.y, light.position.z);

        // ----- allocate object buffer -----
        auto objectAlloc  = frameResource->resourceAllocator->allocate(sizeof(gfx::ObjectBuffer));
        auto objectBuffer = reinterpret_cast<gfx::ObjectBuffer *>(objectAlloc.cpuBase);
        XMStoreFloat4x4(&objectBuffer->world, m_sphere->transform->getLocalToWorld());
        XMStoreFloat4x4(&objectBuffer->inverseWorld, XMMatrixInverse(nullptr, XMLoadFloat4x4(&objectBuffer->world)));
        XMStoreFloat4x4(
            &objectBuffer->transposeInverseWorld, XMMatrixTranspose(XMLoadFloat4x4(&objectBuffer->inverseWorld))
        );
        cmdList->setConstantBufferView(m_graphicsPipeline->getRootParameter("objectBuffer"), objectAlloc.gpuBase);

        // ----- set render resource -----
        XMStoreFloat3(&renderResource->color, XMLoadFloat4(&light.intensity));
        cmdList->set32BitConstants(m_graphicsPipeline->getRootParameter("renderResource"), 4u, (void *)renderResource);

        // -------------- input assembly --------------
        auto mesh = m_sphere->mesh;
        cmdList->setIndexBuffer({
            .bufferLocation = mesh->indexBuffer->resource->GetGPUVirtualAddress(),
            .sizeInBytes    = mesh->indexBufferByteSize,
            .format         = mesh->indexFormat,
        });
        cmdList->setPrimitiveTopology(m_sphere->primitiveTopology);

        // ----- draw submeshes -----
        for (auto &submesh : mesh->submeshes)
        {
            // -------------- draw submesh --------------
            cmdList->drawIndexedInstanced(submesh);
            frameStats->drawCount++;
            frameStats->triangleCount += submesh.indexCount / 3u;
        }
    }
}

void LightDebugRenderPass::initGraphicsPipeline()
{
    std::array<DXGI_FORMAT, 1> formats;
    formats[0] = m_device->getHdrRenderTargetFormat();

    gfx::GraphicsPipelineStateDesc graphicsPipelineState = {
        .vertexShader = {.name = "Debug\\Lights.hlsl", .entryPoint = L"VsMain"},
        .pixelShader  = {.name = "Debug\\Lights.hlsl", .entryPoint = L"PsMain"},
        .rtvCount     = static_cast<UINT32>(formats.size()),
        .rtvFormats   = formats,
        .dsvFormat    = m_device->getDepthStencilFormat(),
        .cullMode     = gfx::CullMode::Back,
        .frontFace    = gfx::FrontFace::Clockwise,
        .depthFunc    = gfx::ComparisonFunc::LessEqual,
    };

    m_graphicsPipeline = std::make_unique<gfx::GraphicsPipeline>(m_device, graphicsPipelineState);
}

} // namespace bisky::renderer