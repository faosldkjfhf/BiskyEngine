#include "Common.hpp"

#include "Core/GameTimer.hpp"
#include "Editor/Editor.hpp"
#include "Graphics/Device.hpp"
#include "Graphics/GraphicsCommandList.hpp"
#include "Graphics/Window.hpp"
#include "Scene/Scene.hpp"

namespace
{

static bisky::editor::HeapAllocator g_heapAllocator;

}

namespace bisky::editor
{

Editor::Editor(gfx::Window *const window, gfx::Device *const device)
{
    g_heapAllocator.Create(
        device->getDevice(),
        std::make_unique<gfx::DescriptorHeap>(
            device->getDevice(), gfx::DescriptorType::CbvSrvUav, 100, gfx::DescriptorFlags::ShaderVisible
        )
    );

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplWin32_Init(window->getHandle());

    ImGui_ImplDX12_InitInfo info{};
    info.Device            = device->getDevice();
    info.CommandQueue      = device->getDirectCommandQueue()->getCommandQueue();
    info.NumFramesInFlight = gfx::Device::FramesInFlight;
    info.RTVFormat         = device->getBackBufferFormat();
    info.DSVFormat = DXGI_FORMAT_UNKNOWN; // FIXME: Figure out how to pass it in, don't really need right now tho
    info.SrvDescriptorHeap    = g_heapAllocator.Heap->getHeap();
    info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo *, D3D12_CPU_DESCRIPTOR_HANDLE *outCPUHandle,
                                   D3D12_GPU_DESCRIPTOR_HANDLE *outGPUHandle) {
        return g_heapAllocator.Allocate(outCPUHandle, outGPUHandle);
    };
    info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo *, D3D12_CPU_DESCRIPTOR_HANDLE outCPUHandle,
                                  D3D12_GPU_DESCRIPTOR_HANDLE outGPUHandle) {
        return g_heapAllocator.Free(outCPUHandle, outGPUHandle);
    };
    ImGui_ImplDX12_Init(&info);
}

Editor::~Editor()
{
    g_heapAllocator.Destroy();
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void Editor::beginFrame()
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void Editor::draw(IRenderable *object)
{
    object->draw();
}

void Editor::endFrame(gfx::GraphicsCommandList *const cmdList, gfx::Device *const device)
{
    ImGui::Render();

    std::array<const gfx::DescriptorHeap *const, 1> heaps = {g_heapAllocator.Heap.get()};
    cmdList->setDescriptorHeaps(heaps);
    cmdList->setRenderTargets(device->getRenderTargetView());
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList->getCommandList());
}

} // namespace bisky::editor