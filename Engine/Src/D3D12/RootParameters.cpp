#include "Common.h"

#include "D3D12/RootParameters.h"

namespace D3D12
{

void DescriptorRange::InitAsShaderResource(UINT baseShaderRegister, UINT numDescriptors, UINT registerSpace,
                                           UINT offsetFromTableStart)
{
  ZeroMemory(&Range, sizeof(D3D12_DESCRIPTOR_RANGE));
  Range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
  Range.BaseShaderRegister = baseShaderRegister;
  Range.NumDescriptors = numDescriptors;
  Range.RegisterSpace = registerSpace;
  Range.OffsetInDescriptorsFromTableStart = offsetFromTableStart;
}

void RootParameters::AddDescriptor(UINT shaderRegister, D3D12_ROOT_PARAMETER_TYPE type, UINT registerSpace,
                                   D3D12_SHADER_VISIBILITY shaderVisibility)
{
  D3D12_ROOT_PARAMETER p{};
  p.ParameterType = type;
  p.Descriptor.ShaderRegister = shaderRegister;
  p.Descriptor.RegisterSpace = registerSpace;
  p.ShaderVisibility = shaderVisibility;
  mParameters.push_back(p);
}

void RootParameters::AddDescriptorTable(const DescriptorRange &DescriptorRange,
                                        D3D12_SHADER_VISIBILITY shaderVisibility)
{
  D3D12_ROOT_PARAMETER p{};
  p.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
  p.ShaderVisibility = shaderVisibility;
  p.DescriptorTable.NumDescriptorRanges = 1;
  p.DescriptorTable.pDescriptorRanges = &DescriptorRange.Range;
  mParameters.push_back(p);
}

} // namespace D3D12