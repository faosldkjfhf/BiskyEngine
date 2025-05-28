#include "Common.h"

#include "Core/Logger.h"
#include "DX12/Context.h"
#include "DX12/Window.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace DX12
{

Window::Error Window::Init(IWindowCallbacks *windowCallbacks, UINT width, UINT height, const std::string &title)
{
  mWindowCallbacks = windowCallbacks;
  mWidth = width;
  mHeight = height;

  WNDCLASSEXW wc{};
  wc.cbSize = sizeof(wc);
  wc.style = CS_OWNDC;
  wc.lpfnWndProc = Window::OnWindowMessage;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = GetModuleHandleW(nullptr);
  wc.hIcon = LoadIconW(nullptr, reinterpret_cast<LPCWSTR>(IDI_APPLICATION));
  wc.hIconSm = LoadIconW(nullptr, reinterpret_cast<LPCWSTR>(IDI_APPLICATION));
  wc.hCursor = LoadCursorW(nullptr, reinterpret_cast<LPCWSTR>(IDC_ARROW));
  wc.hbrBackground = nullptr;
  wc.lpszClassName = L"KevEngine Window Class";
  wc.lpszMenuName = nullptr;
  mWindowClass = RegisterClassExW(&wc);
  if (mWindowClass == 0)
  {
    LOG_ERROR("Window Registration Failed");
    return Error::RegistrationFailed;
  }

  std::string fullTitle = title + " | Bisky Engine 0.1.0";
  std::wstring stemp = std::wstring(fullTitle.begin(), fullTitle.end());
  mWindow = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW, (LPCWSTR)mWindowClass, stemp.c_str(),
                            WS_OVERLAPPEDWINDOW | WS_VISIBLE, 50, 50, mWidth, mHeight, nullptr, nullptr,
                            GetModuleHandle(nullptr), nullptr);
  if (mWindow == nullptr)
  {
    LOG_ERROR("Window Creation Failed");
    return Error::CreationFailed;
  }

  CreateSwapChain();
  CreateHeap();
  GetBuffers();

  LOG_INFO("Win32 Initialized");
  return Error::None;
}

void Window::Shutdown()
{
  ReleaseBuffers();
  mRenderTargetHeap.Reset();
  mSwapChain.Reset();

  if (mWindow)
  {
    DestroyWindow(mWindow);
  }

  if (mWindowClass)
  {
    UnregisterClassW((LPCWSTR)mWindowClass, GetModuleHandleW(nullptr));
  }
}

void Window::Update()
{
  MSG msg;
  while (PeekMessage(&msg, mWindow, 0, 0, PM_REMOVE))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

void Window::Resize()
{
  ReleaseBuffers();

  RECT cr;
  if (GetClientRect(mWindow, &cr))
  {
    mWidth = cr.right - cr.left;
    mHeight = cr.bottom - cr.top;

    mSwapChain->ResizeBuffers(BackBufferCount, mWidth, mHeight, DXGI_FORMAT_UNKNOWN,
                              DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);

    mViewport.Width = static_cast<float>(mWidth);
    mViewport.Height = static_cast<float>(mHeight);
    mScissor.right = mWidth;
    mScissor.bottom = mHeight;

    mShouldResize = false;
  }

  GetBuffers();
}

void Window::BeginFrame(ID3D12GraphicsCommandList10 *cmdList)
{
  mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

  D3D12_RESOURCE_BARRIER transition{};
  transition.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  transition.Transition.pResource = mRenderTargetBuffers[mCurrentBackBufferIndex].Get();
  transition.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
  transition.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
  transition.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  transition.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  cmdList->ResourceBarrier(1, &transition);
}

void Window::EndFrame(ID3D12GraphicsCommandList10 *cmdList)
{
  D3D12_RESOURCE_BARRIER transition{};
  transition.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  transition.Transition.pResource = mRenderTargetBuffers[mCurrentBackBufferIndex].Get();
  transition.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
  transition.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
  transition.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  transition.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  cmdList->ResourceBarrier(1, &transition);
}

void Window::Present()
{
  mSwapChain->Present(0, 0);
}

void Window::SetShouldClose()
{
  mShouldClose = true;
}

LRESULT CALLBACK Window::OnWindowMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
  {
    return true;
  }

  switch (msg)
  {
  case WM_CLOSE:
    Get().mShouldClose = true;
    return 0;
  case WM_SIZE:
    Get().mShouldResize = true;
    break;
  case WM_KEYDOWN:
    Get().mWindowCallbacks->OnKeyDown(wParam);
    break;
  case WM_MOUSEMOVE:
    Get().mWindowCallbacks->OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    break;
  case WM_LBUTTONDOWN: {
    SetCapture(Get().mWindow);
    Get().mWindowCallbacks->OnLeftMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
  }
  break;
  case WM_LBUTTONUP:
    ReleaseCapture();
    Get().mWindowCallbacks->OnLeftMouseUp();
    break;
  default:
    break;
  }
  return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void Window::CreateSwapChain()
{
  DXGI_SWAP_CHAIN_DESC1 sd{};
  sd.Format = mBackBufferFormat;
  sd.Width = mWidth;
  sd.Height = mHeight;
  sd.Stereo = false;
  sd.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.BufferCount = Window::BackBufferCount;
  sd.SampleDesc.Count = 1;
  sd.SampleDesc.Quality = 0;
  sd.Scaling = DXGI_SCALING_STRETCH;
  sd.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
  sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

  DXGI_SWAP_CHAIN_FULLSCREEN_DESC fd{};
  fd.Windowed = true;

  auto &factory = Context::Get().Factory();
  ComPtr<IDXGISwapChain1> sc1;
  factory->CreateSwapChainForHwnd(Context::Get().CommandQueue().Get(), mWindow, &sd, &fd, nullptr, &sc1);
  sc1->QueryInterface(IID_PPV_ARGS(&mSwapChain));

  mViewport.TopLeftX = 0.0f;
  mViewport.TopLeftY = 0.0f;
  mViewport.Width = static_cast<float>(mWidth);
  mViewport.Height = static_cast<float>(mHeight);
  mViewport.MinDepth = 0.0f;
  mViewport.MaxDepth = 1.0f;

  mScissor.left = 0;
  mScissor.top = 0;
  mScissor.right = mWidth;
  mScissor.bottom = mHeight;
}

void Window::CreateHeap()
{
  D3D12_DESCRIPTOR_HEAP_DESC hd{};
  hd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  hd.NumDescriptors = BackBufferCount;
  hd.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  hd.NodeMask = 0;
  Context::Get().Device()->CreateDescriptorHeap(&hd, IID_PPV_ARGS(&mRenderTargetHeap));

  auto handle = mRenderTargetHeap->GetCPUDescriptorHandleForHeapStart();
  auto incrementSize = Context::Get().RtvDescriptorSize();
  for (UINT i = 0; i < BackBufferCount; i++)
  {
    mRenderTargetHandles[i] = handle;
    handle.ptr += incrementSize;
  }
}

Window::Error Window::GetBuffers()
{
  for (UINT i = 0; i < BackBufferCount; i++)
  {
    mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mRenderTargetBuffers[i]));

    D3D12_RENDER_TARGET_VIEW_DESC rtv{};
    rtv.Format = mBackBufferFormat;
    rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    rtv.Texture2D.MipSlice = 0;
    rtv.Texture2D.PlaneSlice = 0;
    Context::Get().Device()->CreateRenderTargetView(mRenderTargetBuffers[i].Get(), &rtv, mRenderTargetHandles[i]);
  }
  return None;
}

void Window::ReleaseBuffers()
{
  for (auto &rt : mRenderTargetBuffers)
  {
    rt.Reset();
  }
}

} // namespace DX12