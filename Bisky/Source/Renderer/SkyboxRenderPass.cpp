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

    initRootSignature();
    initPipelineState();
}

SkyboxRenderPass::~SkyboxRenderPass()
{
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
        cmdList->setPipelineState(m_device->getPipelineState("skyboxRenderPass"));
        cmdList->setRootSignature(m_device->getRootSignature("skyboxRenderPass"));

        gfx::Allocation renderResourceAlloc =
            frameResource->resourceAllocator->allocate(sizeof(SkyboxRenderPass::RenderResource));
        SkyboxRenderPass::RenderResource *renderResource =
            (SkyboxRenderPass::RenderResource *)renderResourceAlloc.cpuBase;
        renderResource->vertexBufferIndex = gfx::Buffer::GetSrvIndex(m_cube->mesh->vertexBuffer.get());
        renderResource->textureIndex      = gfx::Texture::GetSrvIndex(skybox->getTexture());
        cmdList->set32BitConstants(1u, 2u, renderResource);

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
        cmdList->setConstantBufferView(0u, sceneBufferAlloc.gpuBase);

        for (auto &submesh : m_cube->mesh->submeshes)
        {
            cmdList->drawIndexedInstanced(submesh);
        }
    }
}

void SkyboxRenderPass::initRootSignature()
{
    gfx::RootParameters parameters{};
    parameters.addDescriptor(0u, D3D12_ROOT_PARAMETER_TYPE_CBV);
    parameters.add32BitConstants(1u, 2);
    parameters.addStaticSampler({
        .Filter           = D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR,
        .AddressU         = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        .AddressV         = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        .AddressW         = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        .MipLODBias       = 0.0f,
        .ComparisonFunc   = D3D12_COMPARISON_FUNC_ALWAYS,
        .MinLOD           = 0.0f,
        .MaxLOD           = D3D12_FLOAT32_MAX,
        .ShaderRegister   = 0,
        .RegisterSpace    = 0,
        .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL,
    });
    m_device->addRootSignature("skyboxRenderPass", parameters);
}

void SkyboxRenderPass::initPipelineState()
{
    std::array<DXGI_FORMAT, 1> formats;
    formats[0] = m_device->getHdrRenderTargetFormat();

    gfx::GraphicsPipelineStateDesc pso = {
        .rootSignature = m_device->getRootSignature("skyboxRenderPass")->getRootSignature(),
        .vertexShader  = {.name = "RenderPass\\Skybox.hlsl", .entryPoint = L"VsMain"},
        .pixelShader   = {.name = "RenderPass\\Skybox.hlsl", .entryPoint = L"PsMain"},
        .rtvCount      = 1,
        .rtvFormats    = formats,
        .dsvFormat     = m_device->getDepthStencilFormat(),
        .cullMode      = gfx::CullMode::None,
        .frontFace     = gfx::FrontFace::Clockwise,
        .depthFunc     = gfx::ComparisonFunc::LessEqual,
    };
    m_device->addGraphicsPipelineState("skyboxRenderPass", pso);
}

} // namespace bisky::renderer