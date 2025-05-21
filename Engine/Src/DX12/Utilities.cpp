#include "Common.h"

#include "Core/Logger.h"
#include "DX12/Context.h"
#include "DX12/Initializers.h"
#include "DX12/Utilities.h"

UINT DX12::ConstantBufferByteSize(UINT bytes)
{
  return (bytes + 255) & ~255;
}

ComPtr<ID3D12RootSignature> DX12::CreateRootSignature(const DX12::RootParameters &parameters,
                                                      D3D12_ROOT_SIGNATURE_FLAGS flags)
{
  ComPtr<ID3D12RootSignature> rootSignature;

  D3D12_ROOT_SIGNATURE_DESC rs{};
  rs.NumParameters = static_cast<UINT>(parameters.Parameters().size());
  rs.pParameters = parameters.Parameters().data();
  rs.NumStaticSamplers = static_cast<UINT>(Context::Get().StaticSamplers().size());
  rs.pStaticSamplers = Context::Get().StaticSamplers().data();
  rs.Flags = flags;

  ComPtr<ID3DBlob> serializedRootSig = nullptr;
  ComPtr<ID3DBlob> errorBlob = nullptr;
  D3D12SerializeRootSignature(&rs, D3D_ROOT_SIGNATURE_VERSION_1, &serializedRootSig, &errorBlob);
  if (errorBlob != nullptr)
  {
    LOG_WARNING(reinterpret_cast<char *>(errorBlob->GetBufferPointer()));
    return nullptr;
  }

  Context::Get().Device()->CreateRootSignature(0, serializedRootSig->GetBufferPointer(),
                                               serializedRootSig->GetBufferSize(), IID_PPV_ARGS(&rootSignature));

  return rootSignature;
}

ComPtr<ID3D12Resource> DX12::CreateBuffer(ID3D12GraphicsCommandList10 *cmdList, ComPtr<ID3DBlob> &data,
                                          ComPtr<ID3D12Resource> &uploadBuffer)
{
  ComPtr<ID3D12Resource> buffer;

  Context::Get().Device()->CreateCommittedResource(&HeapProperties(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
                                                   &ResourceDesc::Buffer((UINT)data->GetBufferSize()),
                                                   D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&buffer));
  Context::Get().Device()->CreateCommittedResource(
      &HeapProperties(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &ResourceDesc::Buffer((UINT)data->GetBufferSize()),
      D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer));

  void *upload;
  uploadBuffer->Map(0, nullptr, &upload);
  memcpy(upload, data->GetBufferPointer(), data->GetBufferSize());
  uploadBuffer->Unmap(0, nullptr);

  cmdList->ResourceBarrier(
      1, &DX12::ResourceBarrier::Transition(buffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));

  cmdList->CopyBufferRegion(buffer.Get(), 0, uploadBuffer.Get(), 0, data->GetBufferSize());

  cmdList->ResourceBarrier(1, &DX12::ResourceBarrier::Transition(buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
                                                                 D3D12_RESOURCE_STATE_GENERIC_READ));

  return buffer;
}

Ref<DX12::Texture> DX12::CreateTexture(ID3D12GraphicsCommandList10 *cmdList, const DX12::Texture::ImageData &imageData)
{
  UINT32 stride = ((imageData.BitsPerPixel + 7) / 8) * imageData.Width;
  UINT32 size = stride * imageData.Height;
  Ref<DX12::Texture> texture = MakeRef<DX12::Texture>();
  Context::Get().Device()->CreateCommittedResource(
      &HeapProperties(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
      &ResourceDesc::Texture2D(imageData.Width, imageData.Height, imageData.Format), D3D12_RESOURCE_STATE_COPY_DEST,
      nullptr, IID_PPV_ARGS(&texture->Resource));
  Context::Get().Device()->CreateCommittedResource(&HeapProperties(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
                                                   &ResourceDesc::Buffer(size), D3D12_RESOURCE_STATE_GENERIC_READ,
                                                   nullptr, IID_PPV_ARGS(&texture->UploadBuffer));

  void *upload;
  texture->UploadBuffer->Map(0, nullptr, &upload);
  memcpy(upload, imageData.Data.data(), size);
  texture->UploadBuffer->Unmap(0, nullptr);

  D3D12_TEXTURE_COPY_LOCATION src{};
  src.pResource = texture->UploadBuffer.Get();
  src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
  src.PlacedFootprint.Footprint.Width = imageData.Width;
  src.PlacedFootprint.Footprint.Height = imageData.Height;
  src.PlacedFootprint.Footprint.Depth = 1;
  src.PlacedFootprint.Footprint.RowPitch = stride;
  src.PlacedFootprint.Footprint.Format = imageData.Format;
  src.PlacedFootprint.Offset = 0;

  D3D12_TEXTURE_COPY_LOCATION dst{};
  dst.pResource = texture->Resource.Get();
  dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
  dst.SubresourceIndex = 0;

  D3D12_BOX textureBox{};
  textureBox.left = 0;
  textureBox.top = 0;
  textureBox.front = 0;
  textureBox.right = imageData.Width;
  textureBox.bottom = imageData.Height;
  textureBox.back = 1;

  cmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, &textureBox);

  return texture;
}