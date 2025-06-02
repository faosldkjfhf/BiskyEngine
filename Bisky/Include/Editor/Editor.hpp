#pragma once

#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>

#include "Graphics/DescriptorHeap.hpp"

namespace bisky::gfx
{
class Window;
class Device;
class GraphicsCommandList;
} // namespace bisky::gfx

namespace bisky::core
{
class GameTimer;
} // namespace bisky::core

namespace bisky::scene
{
class Scene;
} // namespace bisky::scene

namespace bisky::editor
{

/*
 * Heap allocator for ImGui
 */
struct HeapAllocator
{
    std::unique_ptr<gfx::DescriptorHeap> Heap = nullptr;
    D3D12_DESCRIPTOR_HEAP_TYPE           HeapType;
    D3D12_CPU_DESCRIPTOR_HANDLE          HeapStartCPU;
    D3D12_GPU_DESCRIPTOR_HANDLE          HeapStartGPU;
    UINT                                 HeapHandleIncrement;
    std::vector<int>                     FreeIndices;

    inline void Create(ID3D12Device *device, std::unique_ptr<gfx::DescriptorHeap> heap)
    {
        Heap                            = std::move(heap);
        D3D12_DESCRIPTOR_HEAP_DESC desc = Heap->getHeap()->GetDesc();
        HeapType                        = desc.Type;
        HeapStartCPU                    = Heap->getCpuHeapStart();
        HeapStartGPU                    = Heap->getGpuHeapStart();
        HeapHandleIncrement             = device->GetDescriptorHandleIncrementSize(HeapType);
        FreeIndices.reserve(desc.NumDescriptors);
        for (int n = desc.NumDescriptors; n > 0; n--)
        {
            FreeIndices.push_back(n - 1);
        }
    }

    inline void Destroy()
    {
        Heap = nullptr;
        FreeIndices.clear();
    }

    inline void Allocate(D3D12_CPU_DESCRIPTOR_HANDLE *outCPUDescHandle, D3D12_GPU_DESCRIPTOR_HANDLE *outGPUDescHandle)
    {
        if (FreeIndices.size() == 0)
        {
            throw std::runtime_error("no more free indices");
        }

        int idx = FreeIndices.back();
        FreeIndices.pop_back();
        outCPUDescHandle->ptr = HeapStartCPU.ptr + (idx * HeapHandleIncrement);
        outGPUDescHandle->ptr = HeapStartGPU.ptr + (idx * HeapHandleIncrement);
    }

    inline void Free(D3D12_CPU_DESCRIPTOR_HANDLE outCPUDescHandle, D3D12_GPU_DESCRIPTOR_HANDLE outGPUDescHandle)
    {
        int cpuIdx = (int)((outCPUDescHandle.ptr - HeapStartCPU.ptr) / HeapHandleIncrement);
        int gpuIdx = (int)((outGPUDescHandle.ptr - HeapStartGPU.ptr) / HeapHandleIncrement);
        if (cpuIdx != gpuIdx)
        {
            throw std::runtime_error("cpuIdx != gpuIdx");
        }

        FreeIndices.push_back(cpuIdx);
    }
};

/*
 * ImGui wrapper class.
 *
 * These contain render functions which renders info for
 * various abstractions that I have built.
 */
class Editor
{
  public:
    /*
     * Initialize ImGui for Win32 and D3D12.
     *
     * @param window The window containing the HWND.
     * @param device The d3d12 device context.
     */
    explicit Editor(gfx::Window *const window, gfx::Device *const device);

    /*
     * Shuts down ImGui and releases the global heap allocator.
     */
    ~Editor();

    /*
     * Begins a new frame.
     *
     * This should be called before any render functions
     * or ImGui functions are called.
     */
    void beginFrame();

    /*
     * Renders scene data to a window.
     *
     * @param scene The scene to render data for.
     */
    void render(scene::Scene *const scene);

    /*
     * Ends a draw pass and submits it to the command queue.
     *
     * This should be the final render pass, meaning it should
     * be called once you are done rendering everything else.
     *
     * @param cmdList The graphics command list to submit to.
     * @param device The d3d12 device context.
     */
    void endFrame(gfx::GraphicsCommandList *const cmdList, gfx::Device *const device);

    Editor(const Editor &)                    = delete;
    const Editor &operator=(const Editor &)   = delete;
    Editor(const Editor &&)                   = delete;
    const Editor &&operator=(const Editor &&) = delete;
};

} // namespace bisky::editor