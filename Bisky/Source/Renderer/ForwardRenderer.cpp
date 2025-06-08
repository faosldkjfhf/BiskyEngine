#include "Common.hpp"

#include "Core/FrameStats.hpp"
#include "Graphics/Constants.hpp"
#include "Graphics/Device.hpp"
#include "Graphics/ShaderCompiler.hpp"
#include "Graphics/Window.hpp"
#include "Renderer/ForwardRenderer.hpp"
#include "Scene/Material.hpp"
#include "Scene/Scene.hpp"

namespace bisky::renderer
{

ForwardRenderer::ForwardRenderer(gfx::Window *const window, gfx::Device *const backend) : m_backend(backend)
{
    initRootSignatures();
    initPipelineStateObjects();

    LOG_INFO("Forward Renderer initialized");
}

ForwardRenderer::~ForwardRenderer()
{
}

void ForwardRenderer::draw(
    const RenderLayer &renderLayer, gfx::FrameResource *frameResource, const scene::Scene *const scene,
    core::FrameStats *const frameStats
)
{
    frameStats->drawCount     = 0;
    frameStats->triangleCount = 0;
    auto start                = std::chrono::system_clock::now();

    // -------------- grab the graphics command list --------------
    auto cmdList       = frameResource->graphicsCommandList.get();
    auto renderTexture = m_backend->getHdrRenderTargetBuffer();

    // -------------- clear render target view --------------
    float color[4] = {0.15f, 0.15f, 0.15f, 1.0f};
    cmdList->clearRenderTargetView(renderTexture->rtvDescriptor.cpu, color);
    cmdList->clearDepthStencilView(m_backend->getDepthStencilView(), 1.0f, 0);

    // -------------- set viewport and scissor --------------
    cmdList->setViewport(m_backend->getViewport());
    cmdList->setScissorRect(m_backend->getScissor());

    // -------------- set render targets --------------
    cmdList->setRenderTargets(renderTexture->rtvDescriptor.cpu, m_backend->getDepthStencilView());

    // -------------- bind the pipeline state --------------
    switch (renderLayer)
    {
    case RenderLayer::Skybox:
        break;
    case RenderLayer::Transparent:
        break;
    case RenderLayer::Opaque:
    default:
        cmdList->setPipelineState(m_backend->getPipelineState("opaque"));
        break;
    }

    // -------------- set descriptor heaps --------------
    std::array<const gfx::DescriptorHeap *const, 1> heaps = {m_backend->getCbvSrvUavHeap()};
    cmdList->setDescriptorHeaps(heaps);

    // -------------- bind root signature --------------
    cmdList->setRootSignature(m_backend->getRootSignature("opaque"));

    // -------------- allocate scene buffer --------------
    auto             *camera      = scene->getArcballCamera();
    gfx::Allocation   sceneAlloc  = frameResource->resourceAllocator->allocate(sizeof(gfx::SceneBuffer));
    gfx::SceneBuffer *sceneBuffer = reinterpret_cast<gfx::SceneBuffer *>(sceneAlloc.cpuBase);
    XMStoreFloat4x4(&sceneBuffer->view, camera->getView());
    XMStoreFloat4x4(&sceneBuffer->projection, camera->getProjection());
    XMStoreFloat4x4(&sceneBuffer->viewProjection, camera->getView() * camera->getProjection());
    XMStoreFloat4(&sceneBuffer->viewPosition, camera->getPosition());
    cmdList->setConstantBufferView(0u, sceneAlloc.gpuBase);

    // -------------- allocate lights --------------
    auto             &lights      = scene->getLights();
    gfx::Allocation   alloc       = frameResource->resourceAllocator->allocate(sizeof(gfx::LightBuffer));
    gfx::LightBuffer *lightBuffer = reinterpret_cast<gfx::LightBuffer *>(alloc.cpuBase);
    for (size_t i = 0; i < lights.size(); i++)
    {
        lightBuffer->lights[i] = lights[i];
    }
    lightBuffer->numLights = static_cast<uint32_t>(min(lights.size(), 10));
    cmdList->setConstantBufferView(1u, alloc.gpuBase);

    for (auto &object : scene->getRenderObjects())
    {
        auto *mesh = object->mesh;

        // -------------- input assembly --------------
        cmdList->setIndexBuffer({
            .bufferLocation = mesh->indexBuffer->resource->GetGPUVirtualAddress(),
            .sizeInBytes    = mesh->indexBufferByteSize,
            .format         = mesh->indexFormat,
        });
        cmdList->setPrimitiveTopology(object->primitiveTopology);

        // -------------- allocate render resource --------------
        gfx::Allocation      alloc = frameResource->resourceAllocator->allocate(sizeof(gfx::RenderResource));
        gfx::RenderResource *rr    = (gfx::RenderResource *)alloc.cpuBase;
        rr->vertexBufferIndex      = gfx::Buffer::GetSrvIndex(mesh->vertexBuffer.get());
        rr->sceneBufferIndex       = -1;

        // -------------- allocate object constants --------------
        gfx::Allocation    objectAlloc = frameResource->resourceAllocator->allocate(sizeof(gfx::ObjectBuffer));
        gfx::ObjectBuffer *ptr         = (gfx::ObjectBuffer *)objectAlloc.cpuBase;
        XMStoreFloat4x4(&ptr->world, object->transform->getLocalToWorld());
        XMStoreFloat4x4(&ptr->inverseWorld, dx::XMMatrixInverse(nullptr, object->transform->getLocalToWorld()));
        XMStoreFloat4x4(&ptr->transposeInverseWorld, dx::XMMatrixTranspose(XMLoadFloat4x4(&ptr->inverseWorld)));
        cmdList->setConstantBufferView(2u, objectAlloc.gpuBase);

        for (auto &submesh : mesh->submeshes)
        {
            // ------------- finish setting 32-bit constants -------------
            rr->diffuseTextureIndex           = gfx::Texture::GetSrvIndex(submesh.material->diffuseTexture);
            rr->metallicRoughnessTextureIndex = gfx::Texture::GetSrvIndex(submesh.material->metallicRoughnessTexture);
            rr->normalTextureIndex            = gfx::Texture::GetSrvIndex(submesh.material->normalTexture);
            cmdList->set32BitConstants(3u, 5u, reinterpret_cast<void *>(rr));

            // -------------- draw submesh --------------
            cmdList->drawIndexedInstanced(submesh);
            frameStats->drawCount++;
            frameStats->triangleCount += submesh.indexCount / 3u;
        }
    }

    // -------------- calculate mesh draw time --------------
    auto end                 = std::chrono::system_clock::now();
    auto elapsed             = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    frameStats->meshDrawTime = elapsed.count() / 1000.0f;
}

void ForwardRenderer::initRootSignatures()
{
    gfx::RootParameters parameters{};
    parameters.addDescriptor(0u, D3D12_ROOT_PARAMETER_TYPE_CBV);
    parameters.addDescriptor(1u, D3D12_ROOT_PARAMETER_TYPE_CBV);
    parameters.addDescriptor(2u, D3D12_ROOT_PARAMETER_TYPE_CBV);
    parameters.add32BitConstants(3u, 5u);
    parameters.addStaticSampler({
        .Filter           = D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR,
        .AddressU         = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        .AddressV         = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        .AddressW         = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        .MipLODBias       = 0.0f,
        .ComparisonFunc   = D3D12_COMPARISON_FUNC_ALWAYS,
        .MinLOD           = 0.0f,
        .MaxLOD           = D3D12_FLOAT32_MAX,
        .ShaderRegister   = 0u,
        .RegisterSpace    = 0u,
        .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL,
    });
    m_backend->addRootSignature("opaque", parameters);
}

void ForwardRenderer::initPipelineStateObjects()
{
    std::array<DXGI_FORMAT, 1> formats;
    formats[0] = m_backend->getHdrRenderTargetFormat();

    gfx::GraphicsPipelineStateDesc psoDesc = {
        .rootSignature = m_backend->getRootSignature("opaque")->getRootSignature(),
        .vertexShader  = {.name = "Geometry\\Shader.hlsl", .entryPoint = L"VsMain"},
        .pixelShader   = {.name = "Geometry\\Shader.hlsl", .entryPoint = L"PsMain"},
        .rtvCount      = 1,
        .rtvFormats    = formats,
        .dsvFormat     = m_backend->getDepthStencilFormat(),
        .cullMode      = gfx::CullMode::Back,
        .frontFace     = gfx::FrontFace::Clockwise,
        .depthFunc     = gfx::ComparisonFunc::Less,
    };

    m_backend->addGraphicsPipelineState("opaque", psoDesc);
}

} // namespace bisky::renderer