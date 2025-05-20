#pragma once

#include "Common.h"

namespace Core
{

struct MeshGeometry
{
  struct SubmeshGeometry
  {
    UINT BaseVertexLocation = 0;
    UINT StartIndexLocation = 0;
    UINT IndexCount = 0;
  };

  std::string Name;
  ComPtr<ID3DBlob> VertexBufferCPU;
  ComPtr<ID3DBlob> IndexBufferCPU;
  ComPtr<ID3D12Resource> VertexBufferGPU;
  ComPtr<ID3D12Resource> IndexBufferGPU;
  ComPtr<ID3D12Resource> VertexBufferUploader;
  ComPtr<ID3D12Resource> IndexBufferUploader;

  UINT VertexByteStride = 0;
  UINT VertexBufferByteSize = 0;
  DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
  UINT IndexBufferByteSize = 0;
  std::vector<SubmeshGeometry> DrawArgs;

  inline D3D12_VERTEX_BUFFER_VIEW VertexBufferView() const
  {
    D3D12_VERTEX_BUFFER_VIEW vbv{};
    vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
    vbv.SizeInBytes = VertexBufferByteSize;
    vbv.StrideInBytes = VertexByteStride;
    return vbv;
  }

  inline D3D12_INDEX_BUFFER_VIEW IndexBufferView() const
  {
    D3D12_INDEX_BUFFER_VIEW ibv{};
    ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
    ibv.Format = IndexFormat;
    ibv.SizeInBytes = IndexBufferByteSize;
    return ibv;
  }

  inline void DisposeUploaders()
  {
    VertexBufferUploader = nullptr;
    IndexBufferUploader = nullptr;
  }
};

} // namespace Core
