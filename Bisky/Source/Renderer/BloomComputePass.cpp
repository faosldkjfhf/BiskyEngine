#include "Common.hpp"

#include "Graphics/Device.hpp"
#include "Graphics/Window.hpp"
#include "Renderer/BloomComputePass.hpp"

namespace bisky::renderer
{

BloomComputePass::BloomComputePass(gfx::Device *const device) : m_device(device)
{
    gfx::ComputePipelineStateDesc csDesc = {
        .computeShader = {.name = "RenderPass\\Bloom.hlsl", .entryPoint = L"CsMain"}
    };

    m_computePipeline = std::make_unique<gfx::ComputePipeline>(device, csDesc);
}

void BloomComputePass::draw(gfx::Window *const window, gfx::FrameResource *const frameResource) const
{
    auto cmdList = frameResource->graphicsCommandList.get();

    cmdList->setPipelineState(m_computePipeline->getPipelineState());
    cmdList->setRootSignature(m_computePipeline->getRootSignature());

    // ----- bind bloom texture -----
    // ----- bind output texture -----
    // ----- bind render resource -----
    RenderResource renderResource{};

    // ----- dispatch compute shader -----
    cmdList->dispatch(static_cast<UINT>(window->getWidth() / 8u), static_cast<UINT>(window->getHeight() / 8u), 1u);
}

} // namespace bisky::renderer