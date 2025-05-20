#pragma once

#include "DX12/DescriptorHeap.h"
#include "DX12/FrameResource.h"
#include "DX12/RenderItem.h"

namespace Renderer
{

class ForwardRenderer
{
public:
  enum Error
  {
    None = 0
  };

  ForwardRenderer() = default;
  Error Init();
  void Shutdown();
  void Resize();

  void Draw(ID3D12GraphicsCommandList10 *cmdList, DX12::FrameResource *frameResource,
            const std::vector<Owner<DX12::RenderItem>> &renderItems);

  inline ID3D12RootSignature *RootSignature(std::string_view name)
  {
    auto it = mRootSignatures.find(name);
    if (it == mRootSignatures.end())
    {
      return nullptr;
    }

    return mRootSignatures[name].Get();
  }

  inline ID3D12PipelineState *PipelineStateObject(std::string_view name)
  {
    auto it = mPipelineStateObjects.find(name);
    if (it == mPipelineStateObjects.end())
    {
      return nullptr;
    }

    return mPipelineStateObjects[name].Get();
  }

  inline const D3D12_CPU_DESCRIPTOR_HANDLE &DepthStencilView() const
  {
    return mDepthStencilHandle;
  }

private:
  void InitDepthStencil();
  void InitRootSignatures();
  void InitPipelineStateObjects();

  Owner<DX12::DescriptorHeap> mDepthStencilHeap;
  ComPtr<ID3D12Resource> mDepthStencilBuffer;
  D3D12_CPU_DESCRIPTOR_HANDLE mDepthStencilHandle = {};
  DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

  std::unordered_map<std::string_view, ComPtr<ID3D12PipelineState>> mPipelineStateObjects;
  std::unordered_map<std::string_view, ComPtr<ID3D12RootSignature>> mRootSignatures;
};

} // namespace Renderer
