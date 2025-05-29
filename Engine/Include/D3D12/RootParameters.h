#pragma once

#include "Common.h"

namespace D3D12
{

struct DescriptorRange
{
  void InitAsShaderResource(UINT baseShaderRegister, UINT numDescriptors, UINT registerSpace = 0,
                            UINT offsetFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);

  D3D12_DESCRIPTOR_RANGE Range;
};

class RootParameters
{
public:
  void AddDescriptor(UINT shaderRegister, D3D12_ROOT_PARAMETER_TYPE type, UINT registeSpacer = 0,
                     D3D12_SHADER_VISIBILITY shaderVisibility = D3D12_SHADER_VISIBILITY_ALL);
  void AddDescriptorTable(const DescriptorRange &DescriptorRange,
                          D3D12_SHADER_VISIBILITY shaderVisibility = D3D12_SHADER_VISIBILITY_ALL);

  inline const std::vector<D3D12_ROOT_PARAMETER> &Parameters() const
  {
    return mParameters;
  }

private:
  std::vector<D3D12_ROOT_PARAMETER> mParameters;
};

} // namespace D3D12
