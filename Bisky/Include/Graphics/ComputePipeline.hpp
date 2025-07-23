#pragma once

#include "Graphics/Resources.hpp"
#include <d3d12.h>
#include <string>
#include <unordered_map>
#include <wrl.h>

namespace bisky::gfx
{

class Device;

struct ComputePipelineStateDesc
{
    ShaderModule computeShader;
};

class ComputePipeline
{
  public:
    explicit ComputePipeline(Device *const device, ComputePipelineStateDesc const &desc);

  public:
    ID3D12RootSignature *const getRootSignature() const;
    ID3D12PipelineState *const getPipelineState() const;
    UINT                       getRootParameter(std::string_view name);

  private:
    Device *const                               m_device;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    std::unordered_map<std::string, UINT>       m_rootParameterIndexMap;
};

} // namespace bisky::gfx