#pragma once

#include "Core/Material.h"
#include "Core/MathHelpers.h"
#include "Core/MeshGeometry.h"
#include "Core/Transformation.h"
#include "DX12/Window.h"

namespace DX12
{

struct RenderItem
{
  Ref<Core::MeshGeometry> Geometry = nullptr;
  Ref<Core::Material> Material = nullptr;
  Core::Transformation Transform = {};
  UINT NumFramesDirty = Window::FrameResourceCount;
  D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
  UINT ConstantBufferIndex = 0;
};

} // namespace DX12
