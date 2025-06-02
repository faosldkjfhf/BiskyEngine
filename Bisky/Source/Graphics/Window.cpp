#include "Common.hpp"

#include "Graphics/Device.hpp"
#include "Graphics/Window.hpp"
#include <imgui_impl_win32.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace bisky::gfx
{

Window::Window(uint32_t width, uint32_t height, const std::string &title) : m_width(width), m_height(height)
{
    WNDCLASSEXW wc{};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_OWNDC;
    wc.lpfnWndProc   = Window::_onWindowMessage;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = GetModuleHandleW(nullptr);
    wc.hIcon         = LoadIconW(nullptr, reinterpret_cast<LPCWSTR>(IDI_APPLICATION));
    wc.hIconSm       = LoadIconW(nullptr, reinterpret_cast<LPCWSTR>(IDI_APPLICATION));
    wc.hCursor       = LoadCursorW(nullptr, reinterpret_cast<LPCWSTR>(IDC_ARROW));
    wc.hbrBackground = nullptr;
    wc.lpszClassName = L"Bisky Window Class";
    wc.lpszMenuName  = nullptr;
    m_windowClass    = RegisterClassExW(&wc);
    if (m_windowClass == 0)
    {
        LOG_ERROR("Window Registration Failed");
    }
    LOG_VERBOSE("Window class registered");

    std::string  fullTitle = title + " | Bisky 0.1.0";
    std::wstring stemp     = std::wstring(fullTitle.begin(), fullTitle.end());
    m_window = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW, (LPCWSTR)m_windowClass, stemp.c_str(),
                               WS_OVERLAPPEDWINDOW | WS_VISIBLE, 50, 50, m_width, m_height, nullptr, nullptr,
                               GetModuleHandle(nullptr), nullptr);
    if (m_window == nullptr)
    {
        LOG_ERROR("Window Creation Failed");
    }
    LOG_VERBOSE("Window created");

    SetWindowLongPtrW(m_window, GWLP_USERDATA, (LONG_PTR)this);
    LOG_INFO("Window initialized");
}

Window::~Window()
{
    if (m_window)
    {
        DestroyWindow(m_window);
    }

    if (m_windowClass)
    {
        UnregisterClassW((LPCWSTR)m_windowClass, GetModuleHandleW(nullptr));
    }
}

void Window::update()
{
    MSG msg;
    while (PeekMessage(&msg, m_window, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void Window::resize(Device *const device)
{
    device->releaseBuffers();

    RECT cr;
    if (GetClientRect(m_window, &cr))
    {
        m_width  = cr.right - cr.left;
        m_height = cr.bottom - cr.top;

        device->resize(m_width, m_height);
        LOG_INFO("Window resized to (" + std::to_string(m_width) + ", " + std::to_string(m_height) + ")");
        m_shouldResize = false;
    }

    device->getBuffers(m_width, m_height);
}

void Window::setFullscreenState(bool enabled)
{
    DWORD style   = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    DWORD exStyle = WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW;
    if (enabled)
    {
        style   = WS_POPUP | WS_VISIBLE;
        exStyle = WS_EX_APPWINDOW;
    }

    SetWindowLongW(m_window, GWL_STYLE, style);
    SetWindowLongW(m_window, GWL_EXSTYLE, exStyle);

    if (enabled)
    {
        HMONITOR    monitor = MonitorFromWindow(m_window, MONITOR_DEFAULTTONEAREST);
        MONITORINFO info    = {};
        info.cbSize         = sizeof(info);
        if (GetMonitorInfoW(monitor, &info))
        {
            SetWindowPos(m_window, nullptr, info.rcMonitor.left, info.rcMonitor.top,
                         info.rcMonitor.right - info.rcMonitor.left, info.rcMonitor.bottom - info.rcMonitor.top,
                         SWP_NOZORDER);
            LOG_INFO("Entering fullscreen mode");
        }
    }
    else
    {
        ShowWindow(m_window, SW_MAXIMIZE);
        LOG_INFO("Entering windowed mode");
    }

    m_fullscreenState = enabled;
}

LRESULT CALLBACK Window::onWindowMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CLOSE:
        m_shouldClose = true;
        return 0;
    case WM_SIZE:
        m_shouldResize = true;
        break;
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
        {
            m_shouldClose = true;
            return 0;
        }
        else if (wParam == VK_F11)
        {
            setFullscreenState(!m_fullscreenState);
            break;
        }
        break;
    default:
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK Window::_onWindowMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
    {
        return true;
    }

    Window *window = reinterpret_cast<Window *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (window)
    {
        return window->onWindowMessage(hwnd, msg, wParam, lParam);
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

HWND Window::getHandle()
{
    return m_window;
}

uint32_t Window::getWidth() const
{
    return m_width;
}

uint32_t Window::getHeight() const
{
    return m_height;
}

float Window::getAspectRatio() const
{
    return static_cast<float>(m_width) / m_height;
}

void Window::setShouldClose()
{
    m_shouldClose = true;
}

bool Window::shouldClose() const
{
    return m_shouldClose;
}

bool Window::shouldResize() const
{
    return m_shouldResize;
}

} // namespace bisky::gfx