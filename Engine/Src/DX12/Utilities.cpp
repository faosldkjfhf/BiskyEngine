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
  rs.NumStaticSamplers = 0;
  rs.pStaticSamplers = nullptr;
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