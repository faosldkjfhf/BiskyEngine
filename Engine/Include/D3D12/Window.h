#pragma once

#include "Common.h"
#include "IWindowCallbacks.h"

namespace D3D12
{

class Window
{
public:
  enum Error
  {
    None = 0,
    RegistrationFailed = 1,
    CreationFailed = 2
  };

  Error Init(IWindowCallbacks *windowCallbacks, UINT width = 1920, UINT height = 1080,
             const std::string &title = "Game Demo");
  void Shutdown();
  void Update();
  void Resize();
  void SetFullscreen(bool enabled);
  void BeginFrame(ID3D12GraphicsCommandList10 *cmdList);
  void EndFrame(ID3D12GraphicsCommandList10 *cmdList);
  void Present();
  void SetShouldClose();
  static LRESULT CALLBACK OnWindowMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
  static constexpr UINT BackBufferCount = 2;
  static constexpr UINT FrameResourceCount = 3;

private:
  void CreateSwapChain();
  void CreateHeap();
  Error GetBuffers();
  void ReleaseBuffers();

  HWND mWindow = nullptr;
  ATOM mWindowClass = 0;
  IWindowCallbacks *mWindowCallbacks = nullptr;

  ComPtr<IDXGISwapChain4> mSwapChain;
  ComPtr<ID3D12DescriptorHeap> mRenderTargetHeap;
  ComPtr<ID3D12Resource> mRenderTargetBuffers[BackBufferCount];
  D3D12_CPU_DESCRIPTOR_HANDLE mRenderTargetHandles[BackBufferCount];
  UINT mCurrentBackBufferIndex = 0;

  D3D12_VIEWPORT mViewport;
  D3D12_RECT mScissor;

  DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
  UINT mWidth = 1920;
  UINT mHeight = 1080;
  bool mShouldClose = false;
  bool mShouldResize = false;
  bool mFullscreenEnabled = false;

  POINT mLastMousePos;
  float mDX;
  float mDY;

public:
  Window(const Window &) = delete;
  const Window &operator=(const Window &) = delete;
  inline static Window &Get()
  {
    static Window instance;
    return instance;
  }

  inline HWND Handle()
  {
    return mWindow;
  }

  inline UINT Width() const
  {
    return mWidth;
  }

  inline UINT Height() const
  {
    return mHeight;
  }

  inline float DX() const
  {
    return mDX;
  }

  inline float DY() const
  {
    return mDY;
  }

  inline float AspectRatio() const
  {
    return static_cast<float>(mWidth) / mHeight;
  }

  inline bool ShouldClose() const
  {
    return mShouldClose;
  }

  inline bool ShouldResize() const
  {
    return mShouldResize;
  }

  inline bool FullscreenEnabled() const
  {
    return mFullscreenEnabled;
  }

  inline const D3D12_CPU_DESCRIPTOR_HANDLE &RenderTargetView() const
  {
    return mRenderTargetHandles[mCurrentBackBufferIndex];
  }

  inline const D3D12_VIEWPORT &Viewport() const
  {
    return mViewport;
  }

  inline const D3D12_RECT &Scissor() const
  {
    return mScissor;
  }

  inline DXGI_FORMAT BackBufferFormat() const
  {
    return mBackBufferFormat;
  }

private:
  Window() = default;
};

} // namespace D3D12
