#include "Common.hpp"

#include "Graphics/Constants.hpp"
#include "Graphics/Device.hpp"
#include "Graphics/ShaderCompiler.hpp"
#include "Renderer/ForwardRenderer.hpp"
#include "Scene/Material.hpp"
#include "Scene/Scene.hpp"

namespace bisky::renderer
{

ForwardRenderer::ForwardRenderer(gfx::Device *const backend) : m_backend(backend)
{
    initRootSignatures();
    initPipelineStateObjects();

    LOG_INFO("Forward Renderer initialized");
}

ForwardRenderer::~ForwardRenderer()
{
}

void ForwardRenderer::draw(
    const RenderLayer &renderLayer, gfx::FrameResource *frameResource, const scene::Scene *const scene
)
{
    // -------------- grab the graphics command list --------------
    auto cmdList = frameResource->graphicsCommandList.get();

    // -------------- bind the pipeline state
    cmdList->setPipelineState(m_backend->getPipelineState("opaque"));

    // -------------- set descriptor heaps --------------
    // -------------- must be set before root signature with bindless --------------
    std::array<const gfx::DescriptorHeap *const, 1> heaps = {m_backend->getCbvSrvUavHeap()};
    cmdList->setDescriptorHeaps(heaps);

    // -------------- bind root signature --------------
    cmdList->setRootSignature(m_backend->getRootSignature("opaque"));

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
        rr->sceneBufferIndex       = gfx::Buffer::GetCbvIndex(frameResource->sceneBuffer.get());

        {
            // -------------- allocate object constants --------------
            gfx::Allocation    objectAlloc = frameResource->resourceAllocator->allocate(sizeof(gfx::ObjectBuffer));
            gfx::ObjectBuffer *ptr         = (gfx::ObjectBuffer *)objectAlloc.cpuBase;
            math::XMStoreFloat4x4(&ptr->world, object->transform->getLocalToWorld());
            math::XMStoreFloat4x4(
                &ptr->inverseWorld, math::XMMatrixInverse(nullptr, object->transform->getLocalToWorld())
            );
            cmdList->setConstantBufferView(1, objectAlloc.gpuBase);
        }

        // -------------- draw submeshes --------------
        for (auto &submesh : mesh->submeshes)
        {
            {
                rr->diffuseTextureIndex = gfx::Texture::GetSrvIndex(submesh.material->diffuseTexture);
                rr->metallicRoughnessTextureIndex =
                    gfx::Texture::GetSrvIndex(submesh.material->metallicRoughnessTexture);

                cmdList->set32BitConstants(0, 4u, reinterpret_cast<void *>(rr));
            }

            cmdList->drawIndexedInstanced(submesh);
        }
    }
}

void ForwardRenderer::initRootSignatures()
{
    gfx::RootParameters parameters{};
    parameters.add32BitConstants(0, 4);
    parameters.addDescriptor(1, D3D12_ROOT_PARAMETER_TYPE_CBV);
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
    m_backend->addRootSignature("opaque", parameters);
}

void ForwardRenderer::initPipelineStateObjects()
{
    std::array<DXGI_FORMAT, 1> formats;
    formats[0] = m_backend->getBackBufferFormat();

    gfx::GraphicsPipelineStateDesc psoDesc = {
        .rootSignature = m_backend->getRootSignature("opaque")->getRootSignature(),
        .vertexShader  = {.name = "Geometry\\Shader.hlsl", .entryPoint = L"vertexMain"},
        .pixelShader   = {.name = "Geometry\\Shader.hlsl", .entryPoint = L"pixelMain"},
        .rtvCount      = 1,
        .rtvFormats    = formats,
        .dsvFormat     = m_backend->getDepthStencilFormat(),
        .cullMode      = gfx::CullMode::None,
        .frontFace     = gfx::FrontFace::Clockwise,
        .depthFunc     = gfx::ComparisonFunc::Less,
    };

    m_backend->addGraphicsPipelineState("opaque", psoDesc);
}

} // namespace bisky::renderer