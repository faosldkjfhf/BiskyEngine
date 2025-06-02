#include "Common.hpp"

#include "Graphics/PipelineState.hpp"

namespace bisky::gfx
{

PipelineState::PipelineState(ID3D12Device10 *const device, const GraphicsPipelineStateDesc &gfxD)
{
    constexpr D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {
        .BlendEnable           = false,
        .LogicOpEnable         = false,
        .SrcBlend              = D3D12_BLEND_SRC_ALPHA,
        .DestBlend             = D3D12_BLEND_INV_SRC_ALPHA,
        .BlendOp               = D3D12_BLEND_OP_ADD,
        .SrcBlendAlpha         = D3D12_BLEND_ONE,
        .DestBlendAlpha        = D3D12_BLEND_ZERO,
        .BlendOpAlpha          = D3D12_BLEND_OP_ADD,
        .RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL,
    };

    D3D12_BLEND_DESC blendDesc = {
        .AlphaToCoverageEnable  = false,
        .IndependentBlendEnable = false,
    };

    for (uint32_t i = 0; i < gfxD.rtvCount; i++)
    {
        blendDesc.RenderTarget[i] = renderTargetBlendDesc;
    }

    const D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {
        .DepthEnable      = gfxD.dsvFormat == DXGI_FORMAT_UNKNOWN ? FALSE : TRUE,
        .DepthWriteMask   = D3D12_DEPTH_WRITE_MASK_ALL,
        .DepthFunc        = static_cast<D3D12_COMPARISON_FUNC>(gfxD.depthFunc),
        .StencilEnable    = FALSE,
        .StencilReadMask  = D3D12_DEFAULT_STENCIL_READ_MASK,
        .StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK,
    };

    wrl::ComPtr<IDxcBlob> vs;
    wrl::ComPtr<IDxcBlob> errors;
    if (!ShaderCompiler::compile(ShaderType::Vertex, gfxD.vertexShader.name, gfxD.vertexShader.entryPoint, vs, errors))
    {
        // LOG_WARNING(std::string(static_cast<const char *>(errors->GetBufferPointer())));
    }

    wrl::ComPtr<IDxcBlob> ps;
    if (!ShaderCompiler::compile(ShaderType::Pixel, gfxD.pixelShader.name, gfxD.pixelShader.entryPoint, ps, errors))
    {
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC gps = {
        .pRootSignature = gfxD.rootSignature,
        .VS =
            {
                .pShaderBytecode = reinterpret_cast<BYTE *>(vs->GetBufferPointer()),
                .BytecodeLength  = vs->GetBufferSize(),
            },
        .PS =
            {
                .pShaderBytecode = reinterpret_cast<BYTE *>(ps->GetBufferPointer()),
                .BytecodeLength  = ps->GetBufferSize(),
            },
        .BlendState        = blendDesc,
        .SampleMask        = UINT32_MAX,
        .RasterizerState   = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
        .DepthStencilState = depthStencilDesc,
        .InputLayout =
            {
                .pInputElementDescs = nullptr,
                .NumElements        = 0,
            },
        .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
        .NumRenderTargets      = gfxD.rtvCount,
        .DSVFormat             = gfxD.dsvFormat,
        .SampleDesc            = {.Count = 1u, .Quality = 0u},
        .NodeMask              = 0u,
    };

    gps.RasterizerState.CullMode = static_cast<D3D12_CULL_MODE>(gfxD.cullMode);

    if (gfxD.frontFace == gfx::FrontFace::CounterClockwise)
    {
        gps.RasterizerState.FrontCounterClockwise = true;
    }

    for (uint32_t i = 0; i < gfxD.rtvCount; i++)
    {
        gps.RTVFormats[i] = gfxD.rtvFormats[i];
    }

    device->CreateGraphicsPipelineState(&gps, IID_PPV_ARGS(&m_pipelineState));
}

PipelineState::~PipelineState()
{
    m_pipelineState.Reset();
}

ID3D12PipelineState *const PipelineState::getPipelineState() const
{
    return m_pipelineState.Get();
}

} // namespace bisky::gfx