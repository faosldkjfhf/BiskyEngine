#include "Common.h"

#include "DX12/Context.h"
#include "DX12/Window.h"
#include "Editor/GUI.h"

namespace Editor
{

/*
 * Global heap allocator for ImGui
 */
static HeapAllocator gHeap;

void GUI::Init()
{
#ifdef _DEBUG
  auto heap =
      MakeOwner<DX12::DescriptorHeap>(DX12::DescriptorType::ShaderResource, 64, DX12::DescriptorFlags::ShaderVisible);
  gHeap.Create(DX12::Context::Get().Device().Get(), std::move(heap));

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  ImGui_ImplWin32_Init(DX12::Window::Get().Handle());

  ImGui_ImplDX12_InitInfo info{};
  info.Device = DX12::Context::Get().Device().Get();
  info.CommandQueue = DX12::Context::Get().CommandQueue().Get();
  info.NumFramesInFlight = DX12::Window::FrameResourceCount;
  info.RTVFormat = DX12::Window::Get().BackBufferFormat();
  info.DSVFormat = DXGI_FORMAT_UNKNOWN; // FIXME: Figure out how to pass it in, don't really need right now tho
  info.SrvDescriptorHeap = gHeap.Heap->Resource();
  info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo *, D3D12_CPU_DESCRIPTOR_HANDLE *outCPUHandle,
                                 D3D12_GPU_DESCRIPTOR_HANDLE *outGPUHandle) {
    return gHeap.Allocate(outCPUHandle, outGPUHandle);
  };
  info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo *, D3D12_CPU_DESCRIPTOR_HANDLE outCPUHandle,
                                D3D12_GPU_DESCRIPTOR_HANDLE outGPUHandle) {
    return gHeap.Free(outCPUHandle, outGPUHandle);
  };
  ImGui_ImplDX12_Init(&info);

#endif
}

void GUI::Shutdown()
{
#ifdef _DEBUG
  gHeap.Destroy();
  ImGui_ImplDX12_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();
#endif
}

void GUI::NewFrame(std::function<void()> &&function)
{
#ifdef _DEBUG
  ImGui_ImplDX12_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

  function();
#endif
}

void GUI::Draw(ID3D12GraphicsCommandList *gCmdList)
{
#ifdef _DEBUG
  ImGui::Render();

  ID3D12DescriptorHeap *heaps[] = {gHeap.Heap->Resource()};
  gCmdList->OMSetRenderTargets(1, &DX12::Window::Get().RenderTargetView(), false, nullptr);
  gCmdList->SetDescriptorHeaps(_countof(heaps), heaps);
  ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), gCmdList);
#endif
}

} // namespace Editor