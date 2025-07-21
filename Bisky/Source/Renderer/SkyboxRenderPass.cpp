#include "Common.hpp"

#include "Core/FrameStats.hpp"
#include "Core/ResourceManager.hpp"
#include "Graphics/FrameResource.hpp"
#include "Renderer/SkyboxRenderPass.hpp"
#include "Scene/Scene.hpp"

namespace bisky::renderer
{

SkyboxRenderPass::SkyboxRenderPass(gfx::Device *const device) : m_device(device)
{
    core::ResourceManager::get().loadMesh(device, "Cube\\Cube.gltf");

    m_cube       = std::make_unique<scene::RenderObject>();
    m_cube->name = "skybox";
    m_cube->mesh = core::ResourceManager::get().getMesh("Cube");

    initGraphicsPipeline();
}

SkyboxRenderPass::~SkyboxRenderPass()
{
    m_graphicsPipeline.reset();
    m_cube.reset();
}

void SkyboxRenderPass::draw(
    gfx::FrameResource *const frameResource, scene::Scene *const scene, core::FrameStats *const frameStats
)
{
    auto *cmdList = frameResource->graphicsCommandList.get();
    auto *skybox  = scene->getSkybox();
    auto *camera  = scene->getArcballCamera();
    if (skybox)
    {
        cmdList->setPipelineState(m_graphicsPipeline->getPipelineState());
        cmdList->setRootSignature(m_graphicsPipeline->getRootSignature());

        SkyboxRenderPass::RenderResource renderResource{};
        renderResource.vertexBufferIndex = gfx::Buffer::GetSrvIndex(m_cube->mesh->vertexBuffer.get());
        renderResource.textureIndex      = gfx::Texture::GetSrvIndex(skybox->getTexture());
        cmdList->set32BitConstants(m_graphicsPipeline->getRootParameter("renderResource"), 2u, &renderResource);

        cmdList->setIndexBuffer({
            .bufferLocation = m_cube->mesh->indexBuffer->resource->GetGPUVirtualAddress(),
            .sizeInBytes    = m_cube->mesh->indexBufferByteSize,
            .format         = m_cube->mesh->indexFormat,
        });
        cmdList->setPrimitiveTopology(m_cube->primitiveTopology);

        // TODO: Figure out how to reuse these constants
        gfx::Allocation   sceneBufferAlloc = frameResource->resourceAllocator->allocate(sizeof(gfx::SceneBuffer));
        gfx::SceneBuffer *sceneBuffer      = (gfx::SceneBuffer *)sceneBufferAlloc.cpuBase;
        XMStoreFloat4x4(&sceneBuffer->view, camera->getView());
        XMStoreFloat4x4(&sceneBuffer->projection, camera->getProjection());
        XMStoreFloat4x4(&sceneBuffer->viewProjection, camera->getView() * camera->getProjection());
        XMStoreFloat4(&sceneBuffer->viewPosition, camera->getPosition());
        cmdList->setConstantBufferView(m_graphicsPipeline->getRootParameter("sceneBuffer"), sceneBufferAlloc.gpuBase);

        for (auto &submesh : m_cube->mesh->submeshes)
        {
            cmdList->drawIndexedInstanced(submesh);
        }
    }
}

void SkyboxRenderPass::initGraphicsPipeline()
{
    std::array<DXGI_FORMAT, 1> formats;
    formats[0] = m_device->getHdrRenderTargetFormat();

    gfx::GraphicsPipelineStateDesc pso = {
        .vertexShader = {.name = "RenderPass\\Skybox.hlsl", .entryPoint = L"VsMain"},
        .pixelShader  = {.name = "RenderPass\\Skybox.hlsl", .entryPoint = L"PsMain"},
        .rtvCount     = 1,
        .rtvFormats   = formats,
        .dsvFormat    = m_device->getDepthStencilFormat(),
        .cullMode     = gfx::CullMode::None,
        .frontFace    = gfx::FrontFace::Clockwise,
        .depthFunc    = gfx::ComparisonFunc::LessEqual,
    };

    m_graphicsPipeline = std::make_unique<gfx::GraphicsPipeline>(m_device, pso);
}

} // namespace bisky::renderer