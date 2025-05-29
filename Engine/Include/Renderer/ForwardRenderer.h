#pragma once

#include "D3D12/DescriptorHeap.h"
#include "D3D12/FrameResource.h"
#include "D3D12/RenderItem.h"

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

  void Draw(ID3D12GraphicsCommandList10 *cmdList, D3D12::FrameResource *frameResource,
            const std::vector<Owner<D3D12::RenderItem>> &renderItems);
  void DrawCubeMap(ID3D12GraphicsCommandList10 *cmdList, D3D12::FrameResource *frameResource,
                   const std::vector<Owner<D3D12::RenderItem>> &renderItems, Ref<D3D12::Texture> texture);

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

  Owner<D3D12::DescriptorHeap> mDepthStencilHeap;
  ComPtr<ID3D12Resource> mDepthStencilBuffer;
  D3D12_CPU_DESCRIPTOR_HANDLE mDepthStencilHandle = {};
  DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

  std::unordered_map<std::string_view, ComPtr<ID3D12PipelineState>> mPipelineStateObjects;
  std::unordered_map<std::string_view, ComPtr<ID3D12RootSignature>> mRootSignatures;
};

} // namespace Renderer
