#pragma once

#include "Graphics/Resources.hpp"
#include <d3d12.h>
#include <string>
#include <unordered_map>
#include <wrl.h>

namespace bisky::gfx
{

class Device;
class PipelineState;

class GraphicsPipeline
{
  public:
    explicit GraphicsPipeline(Device *const device, GraphicsPipelineStateDesc const &gfxD);

  public:
    ID3D12RootSignature *const getRootSignature() const;
    ID3D12PipelineState *const getPipelineState() const;
    UINT                       getRootParameter(std::string const &parameterName);

  private:
    Device *const                               m_device;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
    std::unordered_map<std::string, UINT>       m_rootParameterIndexMap;
};

} // namespace bisky::gfx