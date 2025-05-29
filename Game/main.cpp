#include "Core/ArcBall.h"
#include "Core/AssetManager.h"
#include "Core/Constants.h"
#include "Core/GameTimer.h"
#include "Core/GlobalCamera.h"
#include "Core/Logger.h"
#include "Core/MathHelpers.h"
#include "Core/MeshGeometry.h"
#include "Core/Vertex.h"
#include "D3D12/Backend.h"
#include "D3D12/Context.h"
#include "D3D12/DebugLayer.h"
#include "D3D12/FrameResource.h"
#include "D3D12/IWindowCallbacks.h"
#include "D3D12/RenderItem.h"
#include "D3D12/Window.h"
#include "Editor/GUI.h"
#include "Renderer/ForwardRenderer.h"

namespace
{
Owner<D3D12::FrameResource> gFrameResources[D3D12::Window::FrameResourceCount];
UINT gCurrentFrameResourceIndex = 0;

Core::PassConstants gPassConstants;

std::vector<Owner<D3D12::RenderItem>> gRenderItems;
std::vector<Owner<D3D12::RenderItem>> gLights;
std::vector<Owner<D3D12::RenderItem>> gCubemap;
Owner<Renderer::ForwardRenderer> gRenderer;

Core::GameTimer gTimer;
static float gFps = 0.0f;
static float gMspf = 0.0f;
} // namespace

struct Callbacks : D3D12::IWindowCallbacks
{
  inline virtual void OnKeyDown(WPARAM key) override
  {
    switch (key)
    {
    case VK_ESCAPE:
      D3D12::Window::Get().SetShouldClose();
      break;
    case VK_F11:
      D3D12::Window::Get().SetFullscreen(!D3D12::Window::Get().FullscreenEnabled());
      break;
    default:
      break;
    }
  }

  inline virtual void OnLeftMouseDown(WPARAM button, int x, int y) override
  {
    MouseDown = true;
    LastMousePos.x = x;
    LastMousePos.y = y;
    ArcBall.OnLeftMouseDown(x, y);
  }

  inline virtual void OnLeftMouseUp() override
  {
    MouseDown = false;
  }

  inline virtual void OnMouseMove(WPARAM button, int x, int y) override
  {
    if (MouseDown)
    {
      // TODO: Figure out arc ball stuff
      ArcBall.OnMouseMove(x, y);

      // update frame resources
      XMStoreFloat4x4(&gPassConstants.View, Core::GlobalCamera::Get().ViewMatrix());
      XMStoreFloat4(&gPassConstants.ViewPosition, Core::GlobalCamera::Get().Position());
      for (auto &frameResource : gFrameResources)
      {
        frameResource->PassConstants->CopyData(0, gPassConstants);
      }

      LastMousePos.x = x;
      LastMousePos.y = y;
    }
  }

  bool MouseDown = false;
  Core::ArcBall ArcBall;
  POINT LastMousePos;
};

void InitScene(ID3D12GraphicsCommandList10 *cmdList);
void InitFrameResources();
void InitConstants();
void UpdateConstants();
void CalculateFrameStats();
void GUICommands();

int main()
{
  using namespace D3D12;
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

  // FIXME: Loading .glb causes DeadlyImportError - Maybe an assimp thing?
  Backend::ImmediateSubmit([&](ID3D12GraphicsCommandList10 *cmdList) {
    Core::AssetManager::Get().LoadGLTF("sphere.gltf", cmdList);
    Core::AssetManager::Get().LoadGLTF("DamagedHelmet.gltf", cmdList);
    Core::AssetManager::Get().LoadGLTF("Cube.gltf", cmdList);

    // IDEA: Loading cubemap
    // Create 6 RTV OR Load 6 images as textures
    // Render each image to each respective face from a camera centered at the origin of the cube
    // Need a DSV as well
  });

  Core::AssetManager::Get().LoadCubeMap("skybox/cubemap.dds");

  Backend::ImmediateSubmit([&](ID3D12GraphicsCommandList10 *cmdList) {
    InitScene(cmdList);
    InitFrameResources();
    InitConstants();
  });

  Core::AssetManager::Get().DisposeUploaders();

  auto cubemap = Core::AssetManager::Get().GetTexture("skybox/cubemap.dds");

  Window::Get().SetFullscreen(true);
  while (!Window::Get().ShouldClose())
  {
    gCurrentFrameResourceIndex = (gCurrentFrameResourceIndex + 1) % Window::FrameResourceCount;
    auto *frameResource = gFrameResources[gCurrentFrameResourceIndex].get();
    Context::Get().WaitForFence(frameResource->Fence);

    Window::Get().Update();
    UpdateConstants();
    if (Window::Get().ShouldResize())
    {
      frameResource->Fence = Context::Get().AdvanceFence();
      Context::Get().CommandQueue()->Signal(Context::Get().Fence().Get(), frameResource->Fence);
      Context::Get().WaitForFence(frameResource->Fence);

      Window::Get().Resize();
      gRenderer->Resize();
      Core::GlobalCamera::Get().SetLens(90.0f, Window::Get().AspectRatio(), 0.1f, 100.0f);

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
    float clearColor[] = {0.2f, 0.2f, 0.2f, 1.0f};
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
    cmdList->SetGraphicsRootConstantBufferView(6, frameResource->PassConstants->Resource()->GetGPUVirtualAddress());

    // draw using our renderer
    gRenderer->Draw(cmdList, frameResource, gRenderItems);

    // draw the cubemap
    gRenderer->DrawCubeMap(cmdList, frameResource, gCubemap, cubemap);

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

  for (auto &cubemap : gCubemap)
  {
    cubemap.reset();
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
  Core::GlobalCamera::Get().SetPosition(0.0f, 0.0f, 8.0f);

  // materials
  {
    auto mat = Core::AssetManager::Get().AddMaterial("orange");
    mat->Diffuse = XMFLOAT3(1.0f, 0.0f, 0.0f);
    mat->Metallic = 0.5f;
    mat->Roughness = 0.5f;
    mat->AmbientOcclusion = 0.5f;
    mat->NoTexture = true;
  }

  // render items
  {
    auto *ri = gRenderItems.emplace_back(MakeOwner<D3D12::RenderItem>()).get();
    ri->ConstantBufferIndex = 0;
    ri->Geometry = Core::AssetManager::Get().GetModel("Cube_0");
    ri->Material = Core::AssetManager::Get().GetMaterial("Cube_0");
    ri->Transform.SetScale(3.0f, 3.0f, 3.0f);
  }

  // cubemap
  {
    auto *ri = gCubemap.emplace_back(MakeOwner<D3D12::RenderItem>()).get();
    ri->ConstantBufferIndex = static_cast<UINT>(gRenderItems.size() + gLights.size());
    ri->Geometry = Core::AssetManager::Get().GetModel("Cube_0");
  }

  // lights
  {
    auto *ri = gLights.emplace_back(MakeOwner<D3D12::RenderItem>()).get();
    ri->ConstantBufferIndex = static_cast<UINT>(gRenderItems.size()) + 0;
    ri->Geometry = Core::AssetManager::Get().GetModel("sphere_0");
    ri->Material = Core::AssetManager::Get().GetMaterial("sphere_0");
    ri->Transform.SetScale(0.1f, 0.1f, 0.1f);
    ri->Transform.SetTranslation(0.0f, 4.0f, 8.0f);
  }
}

void InitFrameResources()
{
  XMStoreFloat4x4(&gPassConstants.View, Core::GlobalCamera::Get().ViewMatrix());
  XMStoreFloat4x4(&gPassConstants.Projection, Core::GlobalCamera::Get().ProjectionMatrix());
  XMStoreFloat4(&gPassConstants.PointLights[0].Position, FXMVECTOR{0.0f, 4.0f, 8.0f});
  XMStoreFloat4(&gPassConstants.PointLights[0].Strength, FXMVECTOR{1.0f, 1.0f, 1.0f});
  XMStoreFloat4(&gPassConstants.DirectionalLights[0].Direction, FXMVECTOR{0.5f, 0.5f, 0.5f});
  XMStoreFloat4(&gPassConstants.DirectionalLights[0].Strength, FXMVECTOR{1.0f, 1.0f, 1.0f});
  XMStoreFloat4(&gPassConstants.ViewPosition, Core::GlobalCamera::Get().Position());

  for (auto &frameResource : gFrameResources)
  {
    frameResource = MakeOwner<D3D12::FrameResource>(64, 64);
    frameResource->PassConstants->CopyData(0, gPassConstants);
  }
}

void InitConstants()
{
  auto objectCBSize = D3D12::ConstantBufferByteSize(sizeof(Core::ObjectConstants));
  for (UINT i = 0; i < D3D12::Window::FrameResourceCount; i++)
  {
    auto cpuHandle = D3D12::Context::Get().ConstantBufferHeap()->CPUDescriptorHandle();
    UINT idx = D3D12::Context::Get().ConstantBufferHeap()->AddDescriptor();
    cpuHandle.ptr += idx * D3D12::Context::Get().CbvSrvUavDescriptorSize();

    auto cbAddress = gFrameResources[i]->ObjectConstants->Resource()->GetGPUVirtualAddress();
    for (size_t j = 0; j < gRenderItems.size() + gLights.size() + gCubemap.size(); j++)
    {
      D3D12_CONSTANT_BUFFER_VIEW_DESC cbv{};
      cbv.BufferLocation = cbAddress;
      cbv.SizeInBytes = objectCBSize;
      D3D12::Context::Get().Device()->CreateConstantBufferView(&cbv, cpuHandle);
      cbAddress += objectCBSize;
    }
  }

  auto materialCBSize = D3D12::ConstantBufferByteSize(sizeof(Core::MaterialConstants));
  for (UINT i = 0; i < D3D12::Window::FrameResourceCount; i++)
  {
    auto handle = D3D12::Context::Get().ConstantBufferHeap()->CPUDescriptorHandle();
    UINT idx = D3D12::Context::Get().ConstantBufferHeap()->AddDescriptor();
    handle.ptr += idx * D3D12::Context::Get().CbvSrvUavDescriptorSize();

    auto cbAddress = gFrameResources[i]->MaterialConstants->Resource()->GetGPUVirtualAddress();
    for (size_t j = 0; j < Core::AssetManager::Get().NumMaterials(); j++)
    {
      D3D12_CONSTANT_BUFFER_VIEW_DESC cbv{};
      cbv.BufferLocation = cbAddress;
      cbv.SizeInBytes = materialCBSize;
      D3D12::Context::Get().Device()->CreateConstantBufferView(&cbv, handle);
      cbAddress += materialCBSize;
    }
  }

  for (UINT i = 0; i < D3D12::Window::FrameResourceCount; i++)
  {
    auto handle = D3D12::Context::Get().ConstantBufferHeap()->CPUDescriptorHandle();
    UINT idx = D3D12::Context::Get().ConstantBufferHeap()->AddDescriptor();
    handle.ptr += idx * D3D12::Context::Get().CbvSrvUavDescriptorSize();
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbv{};
    cbv.BufferLocation = gFrameResources[i]->PassConstants->Resource()->GetGPUVirtualAddress();
    cbv.SizeInBytes = D3D12::ConstantBufferByteSize(sizeof(Core::PassConstants));
    D3D12::Context::Get().Device()->CreateConstantBufferView(&cbv, handle);
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

      XMMATRIX world = ri->Transform.LocalToWorldMatrix();
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

      XMMATRIX world = ri->Transform.LocalToWorldMatrix();
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
      mat.Metallic = material->Metallic;
      mat.Roughness = material->Roughness;
      mat.AmbientOcclusion = material->AmbientOcclusion;
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
    if (ImGui::SliderFloat3("Position", (float *)&pos, -10.0f, 10.0f))
    {
      Core::GlobalCamera::Get().SetPosition(XMLoadFloat3(&pos));
      XMStoreFloat4x4(&gPassConstants.View, Core::GlobalCamera::Get().ViewMatrix());
      XMStoreFloat4(&gPassConstants.ViewPosition, Core::GlobalCamera::Get().Position());
      for (auto &frameResource : gFrameResources)
      {
        frameResource->PassConstants->CopyData(0, gPassConstants);
      }
    }

    XMFLOAT3 look;
    XMStoreFloat3(&look, Core::GlobalCamera::Get().Look());
    if (ImGui::SliderFloat3("Look", (float *)&look, -10.0f, 10.0f))
    {
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
          material->NumFramesDirty = D3D12::Window::FrameResourceCount;
        }
        if (ImGui::SliderFloat("Metallic", (float *)&material->Metallic, 0.0f, 1.0f))
        {
          material->NumFramesDirty = D3D12::Window::FrameResourceCount;
        }
        if (ImGui::SliderFloat("Roughness", (float *)&material->Roughness, 0.0f, 1.0f))
        {
          material->NumFramesDirty = D3D12::Window::FrameResourceCount;
        }
        if (ImGui::SliderFloat("Ambient Occlusion", (float *)&material->AmbientOcclusion, 0.0f, 1.0f))
        {
          material->NumFramesDirty = D3D12::Window::FrameResourceCount;
        }
        ImGui::Indent();
        ImGui::TreePop();
      }
    }
  }
  if (ImGui::CollapsingHeader("Lights"))
  {
    for (auto &light : gPassConstants.PointLights)
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
          gLights[0]->Transform.AddTranslation(translation.x, translation.y, translation.z);
          gLights[0]->NumFramesDirty = D3D12::Window::FrameResourceCount;

          for (auto &frameResource : gFrameResources)
          {
            frameResource->PassConstants->CopyData(0, gPassConstants);
          }
        }
        ImGui::Indent();
        ImGui::TreePop();
      }
    }
    for (auto &light : gPassConstants.DirectionalLights)
    {
      if (ImGui::TreeNode("Directional Light"))
      {
        ImGui::Unindent();
        auto oldPos = light.Direction;
        if (ImGui::SliderFloat3("Direction", (float *)&light.Direction, -10.0f, 10.0f))
        {
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