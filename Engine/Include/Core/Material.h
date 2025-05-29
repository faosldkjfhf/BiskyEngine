#pragma once

#include "Common.h"
#include "Core/MathHelpers.h"
#include "D3D12/Window.h"

namespace Core
{

struct Material
{
  XMFLOAT3 Diffuse;
  float Metallic = 0.25f;
  float Roughness = 0.25f;
  float AmbientOcclusion = 0.25f;
  UINT DiffuseMapHeapIndex = 0;
  UINT NormalMapHeapIndex = 0;
  UINT AmbientOcclusionMapHeapIndex = 0;
  UINT MetalRoughnessMapHeapIndex = 0;
  UINT ConstantBufferIndex = 0;
  UINT NumFramesDirty = D3D12::Window::FrameResourceCount;
  bool NoTexture = true;
};

} // namespace Core
