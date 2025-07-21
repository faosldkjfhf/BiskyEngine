#include "Common.hpp"

#include "Graphics/Device.hpp"
#include "Graphics/GraphicsPipeline.hpp"
#include "Graphics/PipelineState.hpp"
#include <set>

struct RootParameterComparator
{
    bool operator()(const D3D12_ROOT_PARAMETER &a, const D3D12_ROOT_PARAMETER &b) const
    {
        if (a.ParameterType == D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS)
        {
            if (b.ParameterType == D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS)
            {
                return a.Constants.ShaderRegister < b.Constants.ShaderRegister;
            }
            else
            {
                return a.Constants.ShaderRegister < b.Descriptor.ShaderRegister;
            }
        }
        else
        {
            if (b.ParameterType == D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS)
            {
                return a.Descriptor.ShaderRegister < b.Constants.ShaderRegister;
            }
            else
            {
                return a.Descriptor.ShaderRegister < b.Descriptor.ShaderRegister;
            }
        }
    }
};

namespace bisky::gfx
{

GraphicsPipeline::GraphicsPipeline(Device *const device, GraphicsPipelineStateDesc const &gfxD) : m_device(device)
{
    auto vs = ShaderCompiler::compile(ShaderType::Vertex, gfxD.vertexShader.name, gfxD.vertexShader.entryPoint);
    auto ps = ShaderCompiler::compile(ShaderType::Pixel, gfxD.pixelShader.name, gfxD.pixelShader.entryPoint);
    if (vs.has_value() && ps.has_value())
    {
        // ----- fill root parameter index map -----
        m_rootParameterIndexMap.insert(
            vs.value().rootParameterIndexMap.begin(), vs.value().rootParameterIndexMap.end()
        );
        m_rootParameterIndexMap.insert(
            ps.value().rootParameterIndexMap.begin(), ps.value().rootParameterIndexMap.end()
        );

        // FIXME: This is super scuffed
        // ----- create root signature -----
        std::set<D3D12_ROOT_PARAMETER, RootParameterComparator> rootParameters;
        rootParameters.insert(vs.value().rootParameters.begin(), vs.value().rootParameters.end());
        rootParameters.insert(ps.value().rootParameters.begin(), ps.value().rootParameters.end());
        std::vector<D3D12_ROOT_PARAMETER> orderedParameters(rootParameters.begin(), rootParameters.end());

        constexpr std::array<D3D12_STATIC_SAMPLER_DESC, 1> staticSamplers = {D3D12_STATIC_SAMPLER_DESC{
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
        }};

        D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {
            .NumParameters     = static_cast<UINT>(orderedParameters.size()),
            .pParameters       = orderedParameters.data(),
            .NumStaticSamplers = static_cast<UINT>(staticSamplers.size()),
            .pStaticSamplers   = staticSamplers.data(),
            .Flags             = D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED,
        };

        wrl::ComPtr<ID3DBlob> serialized;
        wrl::ComPtr<ID3DBlob> errors;
        if (FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &serialized, &errors)
            ))
        {
            if (errors && errors->GetBufferSize() > 0)
            {
                LOG_ERROR(std::string(static_cast<char const *>(errors->GetBufferPointer())));
            }
        }

        if (FAILED(device->getDevice()->CreateRootSignature(
                0u, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)
            )))
        {
            LOG_ERROR("Failed to create root signature");
        }

        // ----- create pipeline state -----
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

        D3D12_GRAPHICS_PIPELINE_STATE_DESC gps = {
            .pRootSignature = m_rootSignature.Get(),
            .VS =
                {
                    .pShaderBytecode = reinterpret_cast<BYTE *>(vs.value().shader->GetBufferPointer()),
                    .BytecodeLength  = vs.value().shader->GetBufferSize(),
                },
            .PS =
                {
                    .pShaderBytecode = reinterpret_cast<BYTE *>(ps.value().shader->GetBufferPointer()),
                    .BytecodeLength  = ps.value().shader->GetBufferSize(),
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
        gps.RasterizerState.FillMode = gfxD.fillMode;

        if (gfxD.frontFace == gfx::FrontFace::CounterClockwise)
        {
            gps.RasterizerState.FrontCounterClockwise = true;
        }

        for (uint32_t i = 0; i < gfxD.rtvCount; i++)
        {
            gps.RTVFormats[i] = gfxD.rtvFormats[i];
        }

        if (FAILED(device->getDevice()->CreateGraphicsPipelineState(&gps, IID_PPV_ARGS(&m_pipelineState))))
        {
            LOG_ERROR("Failed to create pipeline state");
        }
    }
}

ID3D12RootSignature *const GraphicsPipeline::getRootSignature() const
{
    return m_rootSignature.Get();
}

ID3D12PipelineState *const GraphicsPipeline::getPipelineState() const
{
    return m_pipelineState.Get();
}

UINT GraphicsPipeline::getRootParameter(std::string const &parameterName)
{
    return m_rootParameterIndexMap[parameterName];
}

} // namespace bisky::gfx