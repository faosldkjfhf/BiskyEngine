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

    initRootSignature();
    initPipelineState();
}

FinalRenderPass::~FinalRenderPass()
{
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
    cmdList->setPipelineState(m_device->getPipelineState("finalRenderPass"));

    // -------------- bind root signature --------------
    cmdList->setRootSignature(m_device->getRootSignature("finalRenderPass"));

    // -------------- allocate constants --------------
    gfx::Allocation allocation = frameResource->resourceAllocator->allocate(sizeof(FinalRenderPass::RenderResource));
    FinalRenderPass::RenderResource *resource = (FinalRenderPass::RenderResource *)allocation.cpuBase;
    resource->vertexBufferIndex               = gfx::Buffer::GetSrvIndex(m_screenQuad->mesh->vertexBuffer.get());
    resource->textureIndex                    = gfx::Texture::GetSrvIndex(m_device->getHdrRenderTargetBuffer());
    cmdList->set32BitConstants(0, 2u, (void *)resource);

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

void FinalRenderPass::initRootSignature()
{
    gfx::RootParameters parameters{};
    parameters.add32BitConstants(0, 2);
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
    m_device->addRootSignature("finalRenderPass", parameters);
}

void FinalRenderPass::initPipelineState()
{
    std::array<DXGI_FORMAT, 1> formats;
    formats[0] = m_device->getBackBufferFormat();

    gfx::GraphicsPipelineStateDesc psoDesc = {
        .rootSignature = m_device->getRootSignature("finalRenderPass")->getRootSignature(),
        .vertexShader  = {.name = "RenderPass\\FinalRenderPass.hlsl", .entryPoint = L"VsMain"},
        .pixelShader   = {.name = "RenderPass\\FinalRenderPass.hlsl", .entryPoint = L"PsMain"},
        .rtvCount      = 1,
        .rtvFormats    = formats,
        .dsvFormat     = DXGI_FORMAT_UNKNOWN,
        .cullMode      = gfx::CullMode::Back,
        .frontFace     = gfx::FrontFace::Clockwise,
        .depthFunc     = gfx::ComparisonFunc::Less,
    };
    m_device->addGraphicsPipelineState("finalRenderPass", psoDesc);
}

} // namespace bisky::renderer