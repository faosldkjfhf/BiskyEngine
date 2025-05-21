#include "Common.h"

#include "Core/AssetManager.h"
#include "Core/Constants.h"
#include "Core/Logger.h"
#include "Core/Vertex.h"
#include "DX12/Context.h"
#include "DX12/RootParameters.h"
#include "DX12/Utilities.h"
#include "DX12/Window.h"
#include "Renderer/ForwardRenderer.h"

namespace Renderer
{

ForwardRenderer::Error ForwardRenderer::Init()
{
  mDepthStencilHeap =
      MakeOwner<DX12::DescriptorHeap>(DX12::DescriptorType::DepthStencil, 2, DX12::DescriptorFlags::None);
  mDepthStencilHandle = mDepthStencilHeap->CPUDescriptorHandle();

  InitDepthStencil();
  InitRootSignatures();
  InitPipelineStateObjects();

  return None;
}

void ForwardRenderer::Shutdown()
{
  for (auto &[_, rootSignature] : mRootSignatures)
  {
    rootSignature.Reset();
  }

  for (auto &[_, pso] : mPipelineStateObjects)
  {
    pso.Reset();
  }

  mDepthStencilBuffer.Reset();
  mDepthStencilHeap.reset();
}

void ForwardRenderer::Resize()
{
  mDepthStencilBuffer.Reset();

  InitDepthStencil();
}

void ForwardRenderer::Draw(ID3D12GraphicsCommandList10 *cmdList, DX12::FrameResource *frameResource,
                           const std::vector<Owner<DX12::RenderItem>> &renderItems)
{
  for (size_t i = 0; i < renderItems.size(); i++)
  {
    auto *ri = renderItems[i].get();

    auto vbv = ri->Geometry->VertexBufferView();
    auto ibv = ri->Geometry->IndexBufferView();
    cmdList->IASetVertexBuffers(0, 1, &vbv);
    cmdList->IASetIndexBuffer(&ibv);
    cmdList->IASetPrimitiveTopology(ri->PrimitiveTopology);

    {
      auto handle = frameResource->ObjectConstants->Resource()->GetGPUVirtualAddress();
      handle += ri->ConstantBufferIndex * DX12::ConstantBufferByteSize(sizeof(Core::ObjectConstants));
      cmdList->SetGraphicsRootConstantBufferView(0, handle);
    }
    {
      auto handle = frameResource->MaterialConstants->Resource()->GetGPUVirtualAddress();
      handle += ri->Material->ConstantBufferIndex * DX12::ConstantBufferByteSize(sizeof(Core::MaterialConstants));
      cmdList->SetGraphicsRootConstantBufferView(1, handle);
    }

    for (auto &submesh : ri->Geometry->DrawArgs)
    {
      cmdList->DrawIndexedInstanced(submesh.IndexCount, 1, submesh.StartIndexLocation, submesh.BaseVertexLocation, 0);
    }
  }
}

void ForwardRenderer::InitDepthStencil()
{
  D3D12_RESOURCE_DESC rd{};
  rd.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  rd.Format = mDepthStencilFormat;
  rd.Width = DX12::Window::Get().Width();
  rd.Height = DX12::Window::Get().Height();
  rd.DepthOrArraySize = 1;
  rd.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
  rd.SampleDesc.Count = 1;
  rd.SampleDesc.Quality = 0;
  rd.MipLevels = 1;
  rd.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  rd.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

  D3D12_CLEAR_VALUE optClear{};
  optClear.DepthStencil.Depth = 1.0f;
  optClear.DepthStencil.Stencil = 0;
  optClear.Format = mDepthStencilFormat;

  DX12::Context::Get().Device()->CreateCommittedResource(&DX12::HeapProperties(D3D12_HEAP_TYPE_DEFAULT),
                                                         D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_DEPTH_WRITE,
                                                         &optClear, IID_PPV_ARGS(&mDepthStencilBuffer));

  UINT idx = mDepthStencilHeap->AddDescriptor();
  D3D12_DEPTH_STENCIL_VIEW_DESC dsv{};
  dsv.Flags = D3D12_DSV_FLAG_NONE;
  dsv.Format = mDepthStencilFormat;
  dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
  dsv.Texture2D.MipSlice = 0;
  DX12::Context::Get().Device()->CreateDepthStencilView(mDepthStencilBuffer.Get(), &dsv, mDepthStencilHandle);
}

void ForwardRenderer::InitRootSignatures()
{
  DX12::RootParameters p{};
  p.AddDescriptor(0, D3D12_ROOT_PARAMETER_TYPE_CBV);
  p.AddDescriptor(1, D3D12_ROOT_PARAMETER_TYPE_CBV);
  p.AddDescriptor(2, D3D12_ROOT_PARAMETER_TYPE_CBV);
  mRootSignatures["opaque"] = DX12::CreateRootSignature(p);
}

void ForwardRenderer::InitPipelineStateObjects()
{
  ComPtr<ID3DBlob> vs = Core::AssetManager::Get().LoadBinary("BlinnPhong\\Vertex.cso");
  ComPtr<ID3DBlob> ps = Core::AssetManager::Get().LoadBinary("BlinnPhong\\Pixel.cso");
  ComPtr<ID3DBlob> lightVS = Core::AssetManager::Get().LoadBinary("Lights\\Vertex.cso");
  ComPtr<ID3DBlob> lightPS = Core::AssetManager::Get().LoadBinary("Lights\\Pixel.cso");

  D3D12_INPUT_ELEMENT_DESC vertexLayout[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Core::Vertex, Position),
       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Core::Vertex, Normal),
       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Core::Vertex, TexCoord),
       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
  };

  D3D12_GRAPHICS_PIPELINE_STATE_DESC gfx;
  ZeroMemory(&gfx, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
  gfx.pRootSignature = mRootSignatures["opaque"].Get();
  gfx.InputLayout.NumElements = _countof(vertexLayout);
  gfx.InputLayout.pInputElementDescs = vertexLayout;
  gfx.StreamOutput.NumEntries = 0;
  gfx.StreamOutput.pSODeclaration = nullptr;
  gfx.StreamOutput.NumStrides = 0;
  gfx.StreamOutput.pBufferStrides = nullptr;
  gfx.StreamOutput.RasterizedStream = 0;
  gfx.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
  gfx.VS.pShaderBytecode = reinterpret_cast<BYTE *>(vs->GetBufferPointer());
  gfx.VS.BytecodeLength = vs->GetBufferSize();
  gfx.PS.pShaderBytecode = reinterpret_cast<BYTE *>(ps->GetBufferPointer());
  gfx.PS.BytecodeLength = ps->GetBufferSize();
  gfx.CachedPSO.CachedBlobSizeInBytes = 0;
  gfx.CachedPSO.pCachedBlob = nullptr;
  gfx.NumRenderTargets = 1;
  gfx.RTVFormats[0] = DX12::Window::Get().BackBufferFormat();
  gfx.DSVFormat = mDepthStencilFormat;
  gfx.DepthStencilState.DepthEnable = TRUE;
  gfx.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
  gfx.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
  gfx.DepthStencilState.StencilEnable = FALSE;
  gfx.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  gfx.RasterizerState.MultisampleEnable = FALSE;
  gfx.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
  gfx.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
  gfx.RasterizerState.FrontCounterClockwise = FALSE;
  gfx.RasterizerState.DepthClipEnable = TRUE;
  gfx.RasterizerState.DepthBias = 0;
  gfx.RasterizerState.DepthBiasClamp = 0.0f;
  gfx.RasterizerState.SlopeScaledDepthBias = 0.0f;
  gfx.RasterizerState.ForcedSampleCount = 0;
  gfx.RasterizerState.AntialiasedLineEnable = FALSE;
  gfx.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
  gfx.BlendState.AlphaToCoverageEnable = FALSE;
  gfx.BlendState.IndependentBlendEnable = FALSE;
  gfx.BlendState.RenderTarget[0].BlendEnable = FALSE;
  gfx.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
  gfx.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
  gfx.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
  gfx.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
  gfx.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
  gfx.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
  gfx.BlendState.RenderTarget[0].LogicOpEnable = FALSE;
  gfx.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
  gfx.SampleMask = 0xFFFFFFFF;
  gfx.SampleDesc.Count = 1;
  gfx.SampleDesc.Quality = 0;
  gfx.NodeMask = 0;
  gfx.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
  DX12::Context::Get().Device()->CreateGraphicsPipelineState(&gfx, IID_PPV_ARGS(&mPipelineStateObjects["opaque"]));

  D3D12_GRAPHICS_PIPELINE_STATE_DESC lights = gfx;
  lights.VS.pShaderBytecode = reinterpret_cast<BYTE *>(lightVS->GetBufferPointer());
  lights.VS.BytecodeLength = lightVS->GetBufferSize();
  lights.PS.pShaderBytecode = reinterpret_cast<BYTE *>(lightPS->GetBufferPointer());
  lights.PS.BytecodeLength = lightPS->GetBufferSize();
  DX12::Context::Get().Device()->CreateGraphicsPipelineState(&lights, IID_PPV_ARGS(&mPipelineStateObjects["lights"]));
}

} // namespace Renderer