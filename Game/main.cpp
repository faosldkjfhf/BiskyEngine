#include "Core/AssetManager.h"
#include "Core/Constants.h"
#include "Core/GameTimer.h"
#include "Core/GlobalCamera.h"
#include "Core/Logger.h"
#include "Core/MeshGeometry.h"
#include "Core/Vertex.h"
#include "DX12/Context.h"
#include "DX12/DebugLayer.h"
#include "DX12/FrameResource.h"
#include "DX12/IWindowCallbacks.h"
#include "DX12/RenderItem.h"
#include "DX12/Window.h"
#include "Editor/GUI.h"
#include "Renderer/ForwardRenderer.h"

struct Callbacks : DX12::IWindowCallbacks
{
  inline virtual void OnKeyDown(WPARAM key) override
  {
    switch (key)
    {
    case VK_ESCAPE:
      DX12::Window::Get().SetShouldClose();
      break;
    default:
      break;
    }
  }

  inline virtual void OnLeftMouseDown(WPARAM button, int x, int y) override
  {
  }

  inline virtual void OnMouseMove(WPARAM button, int x, int y) override
  {
  }
};

namespace
{
Owner<DX12::FrameResource> gFrameResources[DX12::Window::FrameResourceCount];
UINT gCurrentFrameResourceIndex = 0;

Core::PassConstants gPassConstants;

std::vector<Owner<DX12::RenderItem>> gRenderItems;
std::vector<Owner<DX12::RenderItem>> gLights;
Owner<Renderer::ForwardRenderer> gRenderer;

Core::GameTimer gTimer;
static float gFps = 0.0f;
static float gMspf = 0.0f;
} // namespace

void InitScene(ID3D12GraphicsCommandList10 *cmdList);
void InitFrameResources();
void InitConstants();
void UpdateConstants();
void CalculateFrameStats();

int main()
{
  using namespace DX12;
  Callbacks callbacks;

  gTimer.Reset();

  // TODO: Figure out how to automatically set working directory for user
  Core::AssetManager::Get().SetCurrentWorkingDirectory(std::filesystem::absolute(__FILE__).parent_path());
  Core::AssetManager::Get().SetShaderDirectory("Assets/Shaders");
  Core::AssetManager::Get().SetModelDirectory("Assets/Models");

  DebugLayer::Get().Init();
  Context::Get().Init();
  Window::Get().Init(&callbacks);
  gRenderer = MakeOwner<Renderer::ForwardRenderer>();
  gRenderer->Init();
  Editor::GUI::Get().Init();

  // TODO: Make a function that can handle all the command list stuff and have us pass in a lambda?
  auto *cmdList = Context::Get().ResetCommandList();

  Core::AssetManager::Get().LoadGLTF("Box.glb", cmdList);
  Core::AssetManager::Get().LoadGLTF("sphere.gltf", cmdList, fastgltf::Options::LoadExternalBuffers);

  InitScene(cmdList);
  InitFrameResources();
  InitConstants();

  Context::Get().ExecuteCommandList(cmdList);
  Context::Get().FlushCommandQueue();
  Core::AssetManager::Get().DisposeUploaders();

  while (!Window::Get().ShouldClose())
  {
    gCurrentFrameResourceIndex = (gCurrentFrameResourceIndex + 1) % Window::FrameResourceCount;
    auto *frameResource = gFrameResources[gCurrentFrameResourceIndex].get();
    Context::Get().WaitForFence(frameResource->Fence);

    Window::Get().Update();
    UpdateConstants();
    if (Window::Get().ShouldResize())
    {
      Window::Get().Resize();
      gRenderer->Resize();
      XMStoreFloat4x4(&gPassConstants.Projection, Core::GlobalCamera::Get().ProjectionMatrix());
      for (auto &frameResource : gFrameResources)
      {
        frameResource->PassConstants->CopyData(0, gPassConstants);
      }
    }

    // calculate frame statistics
    CalculateFrameStats();

    // begin GUI frame
    Editor::GUI::Get().NewFrame([&]() {
      ImGui::Begin("Debug");
      ImGui::Text("FPS: %f", gFps);
      ImGui::Text("msPF: %f", gMspf);
      ImGui::End();

      ImGui::Begin("Scene");
      if (ImGui::CollapsingHeader("Global Camera"))
      {
        ImGui::SeparatorText("Position");
        XMFLOAT3 pos;
        XMStoreFloat3(&pos, Core::GlobalCamera::Get().Position());
        if (ImGui::SliderFloat("X", (float *)&pos.x, -10.0f, 10.0f) ||
            ImGui::SliderFloat("Y", (float *)&pos.y, -10.0f, 10.0f) ||
            ImGui::SliderFloat("Z", (float *)&pos.z, -10.0f, 10.0f))
        {
          Core::GlobalCamera::Get().SetPosition(XMLoadFloat3(&pos));
          XMStoreFloat4x4(&gPassConstants.View, Core::GlobalCamera::Get().ViewMatrix());
          XMStoreFloat4(&gPassConstants.ViewPosition, Core::GlobalCamera::Get().Position());
          for (auto &frameResource : gFrameResources)
          {
            frameResource->PassConstants->CopyData(0, gPassConstants);
          }
        }
      }
      ImGui::End();
    });

    // reset command list
    auto *cmdList = Context::Get().ResetCommandList(frameResource->CommandAllocator.Get(),
                                                    gRenderer->PipelineStateObject("opaque"));
    Window::Get().BeginFrame(cmdList);

    // clear the render target
    float clearColor[] = {0.0f, 0.0f, 0.0f, 1.0f};
    cmdList->ClearRenderTargetView(Window::Get().RenderTargetView(), clearColor, 0, nullptr);
    cmdList->ClearDepthStencilView(gRenderer->DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
                                   1.0f, 0, 0, nullptr);

    // set the pipeline state
    cmdList->SetPipelineState(gRenderer->PipelineStateObject("opaque"));

    // set the viewport, scissor rect, and render targets
    cmdList->RSSetViewports(1, &Window::Get().Viewport());
    cmdList->RSSetScissorRects(1, &Window::Get().Scissor());
    cmdList->OMSetRenderTargets(1, &Window::Get().RenderTargetView(), false, &gRenderer->DepthStencilView());

    // set the root signature
    cmdList->SetGraphicsRootSignature(gRenderer->RootSignature("opaque"));

    // set the pass constants
    auto handle = frameResource->PassConstants->Resource()->GetGPUVirtualAddress();
    cmdList->SetGraphicsRootConstantBufferView(1, handle);

    // draw using our renderer
    gRenderer->Draw(cmdList, frameResource, gRenderItems);

    // draw the lights
    cmdList->SetPipelineState(gRenderer->PipelineStateObject("lights"));
    gRenderer->Draw(cmdList, frameResource, gLights);

    // draw the GUI
    Editor::GUI::Get().Draw(cmdList);

    // execute command list and present
    Window::Get().EndFrame(cmdList);
    Context::Get().ExecuteCommandList(cmdList);
    Window::Get().Present();

    // signal next fence
    frameResource->Fence = Context::Get().AdvanceFence();
    Context::Get().CommandQueue()->Signal(Context::Get().Fence().Get(), frameResource->Fence);

    gTimer.Tick();
  }

  Context::Get().WaitForFence(gFrameResources[gCurrentFrameResourceIndex]->Fence);
  LOG_INFO("Exiting");

  for (auto &ri : gRenderItems)
  {
    ri.reset();
  }

  for (auto &fr : gFrameResources)
  {
    fr.reset();
  }

  Editor::GUI::Get().Shutdown();
  gRenderer->Shutdown();
  Core::AssetManager::Get().Shutdown();
  Window::Get().Shutdown();
  Context::Get().Shutdown();
  DebugLayer::Get().Shutdown();

  return 0;
}

void InitScene(ID3D12GraphicsCommandList10 *cmdList)
{
  Core::GlobalCamera::Get().SetPosition(FXMVECTOR{3.0f, 3.0f, 3.0f});

  // render items
  {
    auto *ri = gRenderItems.emplace_back(MakeOwner<DX12::RenderItem>()).get();
    ri->ConstantBufferIndex = 0;
    ri->Geometry = Core::AssetManager::Get().GetModel("Mesh");
    XMStoreFloat4x4(&ri->World, XMMatrixScaling(2.0f, 2.0f, 2.0f));
  }

  // lights
  {
    auto *ri = gLights.emplace_back(MakeOwner<DX12::RenderItem>()).get();
    ri->ConstantBufferIndex = static_cast<UINT>(gRenderItems.size()) + 0;
    ri->Geometry = Core::AssetManager::Get().GetModel("sphere.gltf");
    XMStoreFloat4x4(&ri->World, XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixTranslation(0.0f, 3.0f, 3.0f));
  }
}

void InitFrameResources()
{
  XMStoreFloat4x4(&gPassConstants.View, Core::GlobalCamera::Get().ViewMatrix());
  XMStoreFloat4x4(&gPassConstants.Projection, Core::GlobalCamera::Get().ProjectionMatrix());

  auto pos = Core::GlobalCamera::Get().Position();
  XMStoreFloat4(&gPassConstants.ViewPosition, pos);

  for (auto &frameResource : gFrameResources)
  {
    frameResource = MakeOwner<DX12::FrameResource>(static_cast<UINT>(gRenderItems.size() + gLights.size()), 0);
    frameResource->PassConstants->CopyData(0, gPassConstants);
  }
}

void InitConstants()
{
  auto objectCBSize = DX12::ConstantBufferByteSize(sizeof(Core::ObjectConstants));
  for (UINT i = 0; i < DX12::Window::FrameResourceCount; i++)
  {
    auto cpuHandle = DX12::Context::Get().ConstantBufferHeap()->CPUDescriptorHandle();
    UINT idx = DX12::Context::Get().ConstantBufferHeap()->AddDescriptor();
    cpuHandle.ptr += idx * DX12::Context::Get().CbvSrvUavDescriptorSize();

    auto cbAddress = gFrameResources[i]->ObjectConstants->Resource()->GetGPUVirtualAddress();
    for (size_t j = 0; j < gRenderItems.size() + gLights.size(); j++)
    {
      D3D12_CONSTANT_BUFFER_VIEW_DESC cbv{};
      cbv.BufferLocation = cbAddress;
      cbv.SizeInBytes = objectCBSize;
      DX12::Context::Get().Device()->CreateConstantBufferView(&cbv, cpuHandle);
      cbAddress += objectCBSize;
    }
  }
}

void UpdateConstants()
{
  auto *frameResource = gFrameResources[gCurrentFrameResourceIndex].get();

  for (size_t i = 0; i < gRenderItems.size(); i++)
  {
    auto *ri = gRenderItems[i].get();
    if (ri->NumFramesDirty > 0)
    {
      Core::ObjectConstants constants{};

      XMMATRIX world = XMLoadFloat4x4(&ri->World);
      XMStoreFloat4x4(&constants.World, world);
      XMStoreFloat4x4(&constants.InverseWorld, XMMatrixInverse(nullptr, world));
      XMStoreFloat4x4(&constants.NormalMatrix, XMMatrixTranspose(XMMatrixInverse(nullptr, world)));
      frameResource->ObjectConstants->CopyData(static_cast<UINT>(ri->ConstantBufferIndex), constants);
      ri->NumFramesDirty--;
    }
  }

  for (size_t i = 0; i < gLights.size(); i++)
  {
    auto *ri = gLights[i].get();
    if (ri->NumFramesDirty > 0)
    {
      Core::ObjectConstants constants{};

      XMMATRIX world = XMLoadFloat4x4(&ri->World);
      XMStoreFloat4x4(&constants.World, world);
      XMStoreFloat4x4(&constants.InverseWorld, XMMatrixInverse(nullptr, world));
      XMStoreFloat4x4(&constants.NormalMatrix, XMMatrixTranspose(XMMatrixInverse(nullptr, world)));
      frameResource->ObjectConstants->CopyData(static_cast<UINT>(ri->ConstantBufferIndex), constants);
      ri->NumFramesDirty--;
    }
  }
}

void CalculateFrameStats()
{
  static int frameCount = 0;
  static float timeElapsed = 0.0f;

  frameCount++;
  if ((gTimer.GameTime() - timeElapsed) >= 1.0f)
  {
    gFps = (float)frameCount;
    gMspf = 1000.0f / gFps;

    frameCount = 0;
    timeElapsed += 1.0f;
  }
}