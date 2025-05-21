#pragma once

#include "Common.h"
#include "Core/MathHelpers.h"
#include "DX12/Window.h"

namespace Core
{

struct Material
{
  XMFLOAT3 Diffuse;
  UINT ConstantBufferIndex = 0;
  UINT NumFramesDirty = DX12::Window::FrameResourceCount;
};

} // namespace Core
