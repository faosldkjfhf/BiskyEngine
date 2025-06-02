#include "Common.hpp"

#include "Graphics/RootSignature.hpp"

namespace bisky::gfx
{

void RootParameters::add32BitConstants(
    UINT shaderRegister, UINT numConstants, UINT registerSpace, D3D12_SHADER_VISIBILITY shaderVisibility
)
{
    D3D12_ROOT_PARAMETER p     = {};
    p.ParameterType            = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    p.Constants.Num32BitValues = numConstants;
    p.Constants.ShaderRegister = shaderRegister;
    p.Constants.RegisterSpace  = registerSpace;
    p.ShaderVisibility         = shaderVisibility;
    m_rootParameters.push_back(p);
}

void RootParameters::addDescriptor(
    UINT shaderRegister, D3D12_ROOT_PARAMETER_TYPE type, UINT registerSpace, D3D12_SHADER_VISIBILITY shaderVisibility
)
{
    D3D12_ROOT_PARAMETER p      = {};
    p.ParameterType             = type;
    p.Descriptor.ShaderRegister = shaderRegister;
    p.Descriptor.RegisterSpace  = registerSpace;
    p.ShaderVisibility          = shaderVisibility;
    m_rootParameters.push_back(p);
}

void RootParameters::addDescriptorTable(
    const DescriptorRange &DescriptorRange, D3D12_SHADER_VISIBILITY shaderVisibility
)
{
}

void RootParameters::addStaticSampler(const D3D12_STATIC_SAMPLER_DESC desc)
{
    m_staticSamplers.push_back(desc);
}

void RootParameters::clear()
{
    m_rootParameters.clear();
    m_staticSamplers.clear();
}

const std::vector<D3D12_ROOT_PARAMETER> &RootParameters::getRootParameters() const
{
    return m_rootParameters;
}

const std::vector<D3D12_STATIC_SAMPLER_DESC> &RootParameters::getStaticSamplers() const
{
    return m_staticSamplers;
}

RootSignature::RootSignature(ID3D12Device10 *const device, const RootParameters &parameters)
{
    D3D12_ROOT_SIGNATURE_DESC rs{};
    rs.NumParameters     = static_cast<uint32_t>(parameters.getRootParameters().size());
    rs.pParameters       = parameters.getRootParameters().data();
    rs.NumStaticSamplers = static_cast<uint32_t>(parameters.getStaticSamplers().size());
    rs.pStaticSamplers   = parameters.getStaticSamplers().data();
    rs.Flags             = D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;

    wrl::ComPtr<ID3DBlob> serializedRootSig = nullptr;
    wrl::ComPtr<ID3DBlob> errorBlob         = nullptr;
    D3D12SerializeRootSignature(&rs, D3D_ROOT_SIGNATURE_VERSION_1, &serializedRootSig, &errorBlob);
    if (errorBlob != nullptr)
    {
        LOG_WARNING(reinterpret_cast<char *>(errorBlob->GetBufferPointer()));
    }

    device->CreateRootSignature(
        0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)
    );
}

RootSignature::~RootSignature()
{
    m_rootSignature.Reset();
}

ID3D12RootSignature *const RootSignature::getRootSignature() const
{
    return m_rootSignature.Get();
}

} // namespace bisky::gfx