#include "Core/AssetManager.h"
#include "Core/Constants.h"
#include "Core/GameTimer.h"
#include "Core/GlobalCamera.h"
#include "Core/Logger.h"
#include "Core/MeshGeometry.h"
#include "Core/Vertex.h"
#include "DX12/Backend.h"
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
void GUICommands();

int main()
{
  using namespace DX12;
  Callbacks callbacks;

  gTimer.Reset();

  // TODO: Figure out how to automatically set working directory for user
  Core::AssetManager::Get().SetCurrentWorkingDirectory(std::filesystem::absolute(__FILE__).parent_path());
  Core::AssetManager::Get().SetShaderDirectory("Assets/Shaders");
  Core::AssetManager::Get().SetModelDirectory("Assets/Models");
  Core::AssetManager::Get().SetTextureDirectory("Assets/Textures");

  DebugLayer::Get().Init();
  Context::Get().Init();
  Window::Get().Init(&callbacks);
  gRenderer = MakeOwner<Renderer::ForwardRenderer>();
  gRenderer->Init();
  Editor::GUI::Get().Init();

  // TODO: Make a function that can handle all the command list stuff and have us pass in a lambda?
  Backend::ImmediateSubmit([&](ID3D12GraphicsCommandList10 *cmdList) {
    Core::AssetManager::Get().LoadGLTF("sphere.gltf", cmdList, fastgltf::Options::LoadExternalBuffers);
    Core::AssetManager::Get().LoadGLTF("Cube.gltf", cmdList, fastgltf::Options::LoadExternalBuffers);
    Core::AssetManager::Get().LoadGLTF("DamagedHelmet.glb", cmdList);
  });

  Backend::ImmediateSubmit([&](ID3D12GraphicsCommandList10 *cmdList) {
    InitScene(cmdList);
    InitFrameResources();
    InitConstants();
  });

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
    Editor::GUI::Get().NewFrame([&]() { GUICommands(); });

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

    // set descriptor heaps
    ID3D12DescriptorHeap *heaps[] = {Context::Get().ShaderResourceHeap()->Resource()};
    cmdList->SetDescriptorHeaps(_countof(heaps), heaps);

    // set the pass constants
    cmdList->SetGraphicsRootConstantBufferView(3, frameResource->PassConstants->Resource()->GetGPUVirtualAddress());

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

  for (auto &light : gLights)
  {
    light.reset();
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

  // materials
  {
    auto mat = Core::AssetManager::Get().AddMaterial("orange");
    mat->DiffuseMapHeapIndex = Core::AssetManager::Get().GetTexture("Material 2")->HeapIndex;
    mat->NoTexture = false;
    XMStoreFloat3(&mat->Diffuse, FXMVECTOR{1.0f, 0.5f, 0.0f});
  }
  {
    auto mat = Core::AssetManager::Get().AddMaterial("white");
    XMStoreFloat3(&mat->Diffuse, FXMVECTOR{1.0f, 1.0f, 1.0f});
  }

  // render items
  {
    auto *ri = gRenderItems.emplace_back(MakeOwner<DX12::RenderItem>()).get();
    ri->ConstantBufferIndex = 0;
    ri->Geometry = Core::AssetManager::Get().GetModel("mesh_helmet_LP_13930damagedHelmet");
    ri->Material = Core::AssetManager::Get().GetMaterial("orange");
    XMStoreFloat4x4(&ri->World, XMMatrixScaling(1.0f, 1.0f, 1.0f));
  }

  // lights
  {
    auto *ri = gLights.emplace_back(MakeOwner<DX12::RenderItem>()).get();
    ri->ConstantBufferIndex = static_cast<UINT>(gRenderItems.size()) + 0;
    ri->Geometry = Core::AssetManager::Get().GetModel("sphere.gltf");
    ri->Material = Core::AssetManager::Get().GetMaterial("white");
    XMStoreFloat4x4(&ri->World, XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixTranslation(0.0f, 3.0f, 3.0f));
  }
}

void InitFrameResources()
{
  XMStoreFloat4x4(&gPassConstants.View, Core::GlobalCamera::Get().ViewMatrix());
  XMStoreFloat4x4(&gPassConstants.Projection, Core::GlobalCamera::Get().ProjectionMatrix());
  XMStoreFloat4(&gPassConstants.Lights[0].Position, FXMVECTOR{0.0f, 3.0f, 3.0f});
  XMStoreFloat4(&gPassConstants.Lights[0].Strength, FXMVECTOR{1.0f, 1.0f, 1.0f});

  auto pos = Core::GlobalCamera::Get().Position();
  XMStoreFloat4(&gPassConstants.ViewPosition, pos);

  for (auto &frameResource : gFrameResources)
  {
    frameResource = MakeOwner<DX12::FrameResource>(64, 64);
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

  auto materialCBSize = DX12::ConstantBufferByteSize(sizeof(Core::MaterialConstants));
  for (UINT i = 0; i < DX12::Window::FrameResourceCount; i++)
  {
    auto handle = DX12::Context::Get().ConstantBufferHeap()->CPUDescriptorHandle();
    UINT idx = DX12::Context::Get().ConstantBufferHeap()->AddDescriptor();
    handle.ptr += idx * DX12::Context::Get().CbvSrvUavDescriptorSize();

    auto cbAddress = gFrameResources[i]->MaterialConstants->Resource()->GetGPUVirtualAddress();
    for (size_t j = 0; j < Core::AssetManager::Get().NumMaterials(); j++)
    {
      D3D12_CONSTANT_BUFFER_VIEW_DESC cbv{};
      cbv.BufferLocation = cbAddress;
      cbv.SizeInBytes = materialCBSize;
      DX12::Context::Get().Device()->CreateConstantBufferView(&cbv, handle);
      cbAddress += materialCBSize;
    }
  }

  for (UINT i = 0; i < DX12::Window::FrameResourceCount; i++)
  {
    auto handle = DX12::Context::Get().ConstantBufferHeap()->CPUDescriptorHandle();
    UINT idx = DX12::Context::Get().ConstantBufferHeap()->AddDescriptor();
    handle.ptr += idx * DX12::Context::Get().CbvSrvUavDescriptorSize();
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbv{};
    cbv.BufferLocation = gFrameResources[i]->PassConstants->Resource()->GetGPUVirtualAddress();
    cbv.SizeInBytes = DX12::ConstantBufferByteSize(sizeof(Core::PassConstants));
    DX12::Context::Get().Device()->CreateConstantBufferView(&cbv, handle);
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
      frameResource->ObjectConstants->CopyData(ri->ConstantBufferIndex, constants);
      ri->NumFramesDirty--;
    }
  }

  auto &materials = Core::AssetManager::Get().Materials();
  for (auto &[_, material] : materials)
  {
    if (material->NumFramesDirty > 0)
    {
      Core::MaterialConstants mat{};
      mat.Diffuse = material->Diffuse;
      mat.UseMaterial = material->NoTexture;
      frameResource->MaterialConstants->CopyData(material->ConstantBufferIndex, mat);
      material->NumFramesDirty--;
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

void GUICommands()
{
  ImGui::Begin("Debug");
  ImGui::Text("FPS: %f", gFps);
  ImGui::Text("msPF: %f", gMspf);
  ImGui::End();

  ImGui::Begin("Scene");
  if (ImGui::CollapsingHeader("Global Camera"))
  {
    XMFLOAT3 pos;
    XMStoreFloat3(&pos, Core::GlobalCamera::Get().Position());
    if (ImGui::SliderFloat3("Position", (float *)&pos.x, -10.0f, 10.0f))
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
  if (ImGui::CollapsingHeader("Materials"))
  {
    for (auto &[key, material] : Core::AssetManager::Get().Materials())
    {
      auto name = std::string(key);
      if (ImGui::TreeNode(name.c_str()))
      {
        ImGui::Unindent();
        ImGui::SeparatorText("Diffuse");
        if (ImGui::ColorEdit3("Diffuse", (float *)&material->Diffuse,
                              ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_InputRGB))
        {
          material->NumFramesDirty = DX12::Window::FrameResourceCount;
        }
        ImGui::Indent();
        ImGui::TreePop();
      }
    }
  }
  if (ImGui::CollapsingHeader("Lights"))
  {
    for (auto &light : gPassConstants.Lights)
    {
      if (ImGui::TreeNode("Light"))
      {
        ImGui::Unindent();
        auto oldPos = light.Position;
        if (ImGui::SliderFloat3("Position", (float *)&light.Position, -10.0f, 10.0f))
        {
          auto diff = XMVectorSubtract(XMLoadFloat4(&light.Position), XMLoadFloat4(&oldPos));
          XMFLOAT4 translation;
          XMStoreFloat4(&translation, diff);
          XMStoreFloat4x4(&gLights[0]->World, XMLoadFloat4x4(&gLights[0]->World) *
                                                  XMMatrixTranslation(translation.x, translation.y, translation.z));
          gLights[0]->NumFramesDirty = DX12::Window::FrameResourceCount;

          for (auto &frameResource : gFrameResources)
          {
            frameResource->PassConstants->CopyData(0, gPassConstants);
          }
        }
        ImGui::Indent();
        ImGui::TreePop();
      }
    }
  }
  ImGui::End();
}