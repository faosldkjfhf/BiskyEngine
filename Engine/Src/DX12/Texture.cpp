#include "Common.h"

#include "DX12/Context.h"
#include "DX12/Texture.h"

namespace DX12
{

void Texture::CreateView()
{
  auto handle = Context::Get().ShaderResourceHeap()->CPUDescriptorHandle();
  HeapIndex = Context::Get().ShaderResourceHeap()->AddDescriptor();
  handle.ptr += HeapIndex * Context::Get().CbvSrvUavDescriptorSize();

  D3D12_SHADER_RESOURCE_VIEW_DESC srv{};
  srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
  srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  srv.Format = Resource->GetDesc().Format;
  srv.Texture2D.MipLevels = Resource->GetDesc().MipLevels;
  srv.Texture2D.MostDetailedMip = 0;
  srv.Texture2D.PlaneSlice = 0;
  srv.Texture2D.ResourceMinLODClamp = 0.0f;
  Context::Get().Device()->CreateShaderResourceView(Resource.Get(), &srv, handle);
}

void Texture::DisposeUploader()
{
  UploadBuffer = nullptr;
}

} // namespace DX12