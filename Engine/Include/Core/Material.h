#pragma once

#include "Common.h"
#include "Core/MathHelpers.h"
#include "DX12/Window.h"

namespace Core
{

struct Material
{
  XMFLOAT3 Diffuse;
  UINT DiffuseMapHeapIndex = 0;
  UINT ConstantBufferIndex = 0;
  UINT NumFramesDirty = DX12::Window::FrameResourceCount;
  bool NoTexture = true;
};

} // namespace Core
