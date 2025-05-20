#pragma once

#include "Core/Constants.h"
#include "DX12/UploadBuffer.h"

namespace DX12
{

struct FrameResource
{
  FrameResource(UINT numObjects, UINT numMaterials);

  UINT64 Fence = 0;
  ComPtr<ID3D12CommandAllocator> CommandAllocator = nullptr;
  Owner<UploadBuffer<Core::ObjectConstants>> ObjectConstants = nullptr;
  Owner<UploadBuffer<Core::PassConstants>> PassConstants = nullptr;
};

} // namespace DX12
