#pragma once

#include "Common.hpp"

namespace bisky::gfx
{

class DescriptorRange
{
};

enum RootSignatureParameters
{
    PassConstantBufferView,
    ObjectConstantBufferView,
    RenderResourceConstantBufferView,
};

class RootParameters
{
  public:
    explicit RootParameters() = default;
    ~RootParameters()         = default;

  public:
    void add32BitConstants(UINT shaderRegister, UINT numConstants, UINT registerSpace = 0,
                           D3D12_SHADER_VISIBILITY shaderVisibility = D3D12_SHADER_VISIBILITY_ALL);
    void addDescriptor(UINT shaderRegister, D3D12_ROOT_PARAMETER_TYPE type, UINT registerSpace = 0,
                       D3D12_SHADER_VISIBILITY shaderVisibility = D3D12_SHADER_VISIBILITY_ALL);
    void addDescriptorTable(const DescriptorRange  &DescriptorRange,
                            D3D12_SHADER_VISIBILITY shaderVisibility = D3D12_SHADER_VISIBILITY_ALL);
    void addStaticSampler(const D3D12_STATIC_SAMPLER_DESC desc);
    void clear();

  public:
    const std::vector<D3D12_ROOT_PARAMETER>      &getRootParameters() const;
    const std::vector<D3D12_STATIC_SAMPLER_DESC> &getStaticSamplers() const;

  private:
    std::vector<D3D12_ROOT_PARAMETER>      m_rootParameters;
    std::vector<D3D12_STATIC_SAMPLER_DESC> m_staticSamplers;
};

/*
 * A wrapper around a d3d12 root signature.
 */
class RootSignature
{
  public:
    explicit RootSignature(ID3D12Device10 *const device, const RootParameters &parameters);
    ~RootSignature();

    RootSignature(const RootSignature &)                    = delete;
    const RootSignature &operator=(const RootSignature &)   = delete;
    RootSignature(const RootSignature &&)                   = delete;
    const RootSignature &&operator=(const RootSignature &&) = delete;

  public:
    ID3D12RootSignature *const getRootSignature() const;

  private:
    wrl::ComPtr<ID3D12RootSignature> m_rootSignature;
};

} // namespace bisky::gfx