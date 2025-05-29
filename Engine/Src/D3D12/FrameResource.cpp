#include "Common.h"

#include "D3D12/Context.h"
#include "D3D12/FrameResource.h"

namespace D3D12
{

FrameResource::FrameResource(UINT numObjects, UINT numMaterials)
{
  Fence = 0;
  Context::Get().Device()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CommandAllocator));
  ObjectConstants = MakeOwner<UploadBuffer<Core::ObjectConstants>>(numObjects);
  MaterialConstants = MakeOwner<UploadBuffer<Core::MaterialConstants>>(numMaterials);
  PassConstants = MakeOwner<UploadBuffer<Core::PassConstants>>(1);
}

} // namespace D3D12