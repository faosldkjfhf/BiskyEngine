#include "Common.hpp"

#include "Core/FrameStats.hpp"
#include "Graphics/Constants.hpp"
#include "Graphics/Device.hpp"
#include "Graphics/Material.hpp"
#include "Graphics/ShaderCompiler.hpp"
#include "Graphics/Window.hpp"
#include "Renderer/ForwardRenderer.hpp"
#include "Scene/Scene.hpp"

namespace bisky::renderer
{

ForwardRenderer::ForwardRenderer(gfx::Window *const window, gfx::Device *const backend) : m_backend(backend)
{
    initPipeline();
    LOG_INFO("Forward Renderer initialized");
}

ForwardRenderer::~ForwardRenderer()
{
    m_graphicsPipeline.reset();
}

void ForwardRenderer::draw(
    const RenderLayer &renderLayer, gfx::FrameResource *frameResource, const scene::Scene *const scene,
    core::FrameStats *const frameStats
)
{
    frameStats->drawCount     = 0u;
    frameStats->triangleCount = 0u;
    auto start                = std::chrono::system_clock::now();

    // -------------- grab the graphics command list --------------
    auto cmdList       = frameResource->graphicsCommandList.get();
    auto renderTexture = m_backend->getHdrRenderTargetBuffer();

    // -------------- clear render target view --------------
    float color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    cmdList->clearRenderTargetView(renderTexture->rtvDescriptor.cpu, color);
    cmdList->clearDepthStencilView(m_backend->getDepthStencilView(), 1.0f, 0);

    // -------------- set viewport and scissor --------------
    cmdList->setViewport(m_backend->getViewport());
    cmdList->setScissorRect(m_backend->getScissor());

    // -------------- set render targets --------------
    cmdList->setRenderTargets(renderTexture->rtvDescriptor.cpu, m_backend->getDepthStencilView());

    // -------------- bind the pipeline state --------------
    cmdList->setPipelineState(m_graphicsPipeline->getPipelineState());

    // -------------- set descriptor heaps --------------
    std::array<const gfx::DescriptorHeap *const, 1> heaps = {m_backend->getCbvSrvUavHeap()};
    cmdList->setDescriptorHeaps(heaps);

    // -------------- bind root signature --------------
    cmdList->setRootSignature(m_graphicsPipeline->getRootSignature());

    // -------------- bind scene buffer --------------
    cmdList->setConstantBufferView(
        m_graphicsPipeline->getRootParameter("sceneBuffer"),
        frameResource->sceneBuffer->resource->GetGPUVirtualAddress()
    );

    // -------------- allocate lights --------------
    auto             &lights      = scene->getLights();
    gfx::Allocation   alloc       = frameResource->resourceAllocator->allocate(sizeof(gfx::LightBuffer));
    gfx::LightBuffer *lightBuffer = reinterpret_cast<gfx::LightBuffer *>(alloc.cpuBase);
    for (const UINT32 i : std::views::iota(0u, lights.size()))
    {
        lightBuffer->lights[i] = lights[i];
    }
    lightBuffer->numLights = static_cast<uint32_t>(min(lights.size(), 10));
    cmdList->setConstantBufferView(m_graphicsPipeline->getRootParameter("lightBuffer"), alloc.gpuBase);

    for (auto &object : scene->getRenderObjects())
    {
        auto mesh = object->mesh;

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

        // -------------- allocate object constants --------------
        gfx::Allocation    objectAlloc = frameResource->resourceAllocator->allocate(sizeof(gfx::ObjectBuffer));
        gfx::ObjectBuffer *ptr         = (gfx::ObjectBuffer *)objectAlloc.cpuBase;
        XMStoreFloat4x4(&ptr->world, object->transform->getLocalToWorld());
        XMStoreFloat4x4(&ptr->inverseWorld, dx::XMMatrixInverse(nullptr, object->transform->getLocalToWorld()));
        XMStoreFloat4x4(&ptr->transposeInverseWorld, dx::XMMatrixTranspose(XMLoadFloat4x4(&ptr->inverseWorld)));
        cmdList->setConstantBufferView(m_graphicsPipeline->getRootParameter("objectBuffer"), objectAlloc.gpuBase);

        for (auto &submesh : mesh->submeshes)
        {
            // ------------- finish setting 32-bit constants -------------
            rr->diffuseTextureIndex = gfx::Texture::GetSrvIndex(submesh.material->diffuseTexture.get());
            rr->metallicRoughnessTextureIndex =
                gfx::Texture::GetSrvIndex(submesh.material->metallicRoughnessTexture.get());
            rr->normalTextureIndex = gfx::Texture::GetSrvIndex(submesh.material->normalTexture.get());
            cmdList->set32BitConstants(
                m_graphicsPipeline->getRootParameter("renderResource"), 4u, reinterpret_cast<void *>(rr)
            );

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

void ForwardRenderer::initPipeline()
{
    std::array<DXGI_FORMAT, 1> formats;
    formats[0] = m_backend->getHdrRenderTargetFormat();

    gfx::GraphicsPipelineStateDesc psoDesc = {
        .vertexShader = {.name = "Geometry\\Shader.hlsl", .entryPoint = L"VsMain"},
        .pixelShader  = {.name = "Geometry\\Shader.hlsl", .entryPoint = L"PsMain"},
        .rtvCount     = 1u,
        .rtvFormats   = formats,
        .dsvFormat    = m_backend->getDepthStencilFormat(),
        .cullMode     = gfx::CullMode::Back,
        .frontFace    = gfx::FrontFace::Clockwise,
        .depthFunc    = gfx::ComparisonFunc::Less,
    };

    m_graphicsPipeline = std::make_unique<gfx::GraphicsPipeline>(m_backend, psoDesc);
}

} // namespace bisky::renderer