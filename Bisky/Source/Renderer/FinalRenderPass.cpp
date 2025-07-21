#include "Common.hpp"

#include "Core/FrameStats.hpp"
#include "Core/ResourceManager.hpp"
#include "Graphics/Device.hpp"
#include "Graphics/ShaderCompiler.hpp"
#include "Graphics/Texture.hpp"
#include "Renderer/FinalRenderPass.hpp"
#include "Scene/ScreenQuad.hpp"

namespace bisky::renderer
{

FinalRenderPass::FinalRenderPass(gfx::Device *const device) : m_device(device)
{
    m_screenQuad = std::make_unique<scene::RenderObject>();

    core::ResourceManager::get().addMesh(scene::ScreenQuad::mesh(device));
    m_screenQuad->mesh = core::ResourceManager::get().getMesh("ScreenQuad");

    initGraphicsPipeline();
}

FinalRenderPass::~FinalRenderPass()
{
    m_graphicsPipeline.reset();
    m_screenQuad.reset();
}

void FinalRenderPass::draw(gfx::FrameResource *const frameResource, core::FrameStats *const frameStats)
{
    auto  start   = std::chrono::system_clock::now();
    auto *cmdList = frameResource->graphicsCommandList.get();
    auto *mesh    = m_screenQuad->mesh;

    // -------------- clear render target --------------
    float color[4] = {0.15f, 0.15f, 0.15f, 1.0f};
    cmdList->clearRenderTargetView(m_device->getRenderTargetView(), color);

    // -------------- set render targets --------------
    cmdList->setRenderTargets(m_device->getRenderTargetView());

    // -------------- set pipeline state --------------
    cmdList->setPipelineState(m_graphicsPipeline->getPipelineState());

    // -------------- bind root signature --------------
    cmdList->setRootSignature(m_graphicsPipeline->getRootSignature());

    // -------------- allocate constants --------------
    FinalRenderPass::RenderResource renderResource{};
    renderResource.vertexBufferIndex = gfx::Buffer::GetSrvIndex(m_screenQuad->mesh->vertexBuffer.get());
    renderResource.textureIndex      = gfx::Texture::GetSrvIndex(m_device->getHdrRenderTargetBuffer());
    cmdList->set32BitConstants(m_graphicsPipeline->getRootParameter("renderResource"), 2u, (void *)&renderResource);

    // -------------- input assembly --------------
    cmdList->setIndexBuffer({
        .bufferLocation = mesh->indexBuffer->resource->GetGPUVirtualAddress(),
        .sizeInBytes    = mesh->indexBufferByteSize,
        .format         = mesh->indexFormat,
    });
    cmdList->setPrimitiveTopology(m_screenQuad->primitiveTopology);

    // -------------- draw --------------
    for (auto &submesh : mesh->submeshes)
    {
        cmdList->drawIndexedInstanced(submesh);
    }

    auto end                        = std::chrono::system_clock::now();
    auto elapsed                    = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    frameStats->finalRenderDrawTime = elapsed.count() / 1000.0f;
}

void FinalRenderPass::initGraphicsPipeline()
{
    std::array<DXGI_FORMAT, 1> formats;
    formats[0] = m_device->getBackBufferFormat();

    gfx::GraphicsPipelineStateDesc psoDesc = {
        .vertexShader = {.name = "RenderPass\\FinalRenderPass.hlsl", .entryPoint = L"VsMain"},
        .pixelShader  = {.name = "RenderPass\\FinalRenderPass.hlsl", .entryPoint = L"PsMain"},
        .rtvCount     = 1u,
        .rtvFormats   = formats,
        .dsvFormat    = DXGI_FORMAT_UNKNOWN,
        .cullMode     = gfx::CullMode::Back,
        .frontFace    = gfx::FrontFace::Clockwise,
        .depthFunc    = gfx::ComparisonFunc::Less,
    };

    m_graphicsPipeline = std::make_unique<gfx::GraphicsPipeline>(m_device, psoDesc);
}

} // namespace bisky::renderer