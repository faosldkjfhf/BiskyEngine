#include "Common.hpp"

#include "Core/ResourceManager.hpp"
#include "Graphics/ComputePipeline.hpp"
#include "Graphics/Device.hpp"

namespace bisky::gfx
{

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

ComputePipeline::ComputePipeline(Device *const device, ComputePipelineStateDesc const &desc) : m_device(device)
{
    // ----- compile shader -----
    auto cs = ShaderCompiler::compile(ShaderType::Compute, desc.computeShader.name, desc.computeShader.entryPoint);
    if (cs.has_value())
    {
        // ----- fill root parameters -----
        m_rootParameterIndexMap.insert(
            cs.value().rootParameterIndexMap.begin(), cs.value().rootParameterIndexMap.end()
        );

        // ----- create root signature -----
        std::set<D3D12_ROOT_PARAMETER, RootParameterComparator> rootParameters;
        rootParameters.insert(cs.value().rootParameters.begin(), cs.value().rootParameters.end());
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
        D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineDesc = {
            .pRootSignature = m_rootSignature.Get(),
            .CS =
                {
                    .pShaderBytecode = reinterpret_cast<BYTE *>(cs.value().shader->GetBufferPointer()),
                    .BytecodeLength  = cs.value().shader->GetBufferSize(),
                },
            .NodeMask = 0u,
        };

        if (FAILED(device->getDevice()->CreateComputePipelineState(&computePipelineDesc, IID_PPV_ARGS(&m_pipelineState))
            ))
        {
            LOG_ERROR("Failed to create pipeline state");
        }

        LOG_INFO("Created compute pipeline");
    }
}

ID3D12RootSignature *const ComputePipeline::getRootSignature() const
{
    return m_rootSignature.Get();
}

ID3D12PipelineState *const ComputePipeline::getPipelineState() const
{
    return m_pipelineState.Get();
}

UINT ComputePipeline::getRootParameter(std::string_view name)
{
    return m_rootParameterIndexMap[std::string(name)];
}

} // namespace bisky::gfx