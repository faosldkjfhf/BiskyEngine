#pragma once

#include "Core/Constants.h"
#include "D3D12/UploadBuffer.h"

namespace D3D12
{

struct FrameResource
{
  FrameResource(UINT numObjects, UINT numMaterials);

  UINT64 Fence = 0;
  ComPtr<ID3D12CommandAllocator> CommandAllocator = nullptr;
  Owner<UploadBuffer<Core::ObjectConstants>> ObjectConstants = nullptr;
  Owner<UploadBuffer<Core::MaterialConstants>> MaterialConstants = nullptr;
  Owner<UploadBuffer<Core::PassConstants>> PassConstants = nullptr;
};

} // namespace D3D12
