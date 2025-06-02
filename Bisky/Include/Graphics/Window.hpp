#pragma once

#include "Common.hpp"

namespace bisky::gfx
{

/*
 * A wrapper around a Win32 window class.
 *
 * TODO: Factor out the window messages so the user can define them.
 */
class Window
{
  public:
    /*
     * Registers a new window class and creates a Win32 window.
     *
     * @param width The starting width.
     * @param height The starting height.
     * @param title The title of the window.
     */
    explicit Window(uint32_t width, uint32_t height, const std::string &title);

    /*
     * Unregisters the window class and destroys the window.
     */
    ~Window();

    Window(const Window &)                    = delete;
    const Window &operator=(const Window &)   = delete;
    Window(const Window &&)                   = delete;
    const Window &&operator=(const Window &&) = delete;

  public:
    /*
     * Translates and dispatches all pending window messages.
     */
    void update();

    /*
     * Resizes the window and the device.
     *
     * @param device The graphics device to resize.
     */
    void resize(Device *const device);

    /*
     * Toggles the window between fullscreen mode and windowed mode.
     *
     * @param enabled set to fullscreen if true.
     */
    void setFullscreenState(bool enabled);

    /*
     * This method will handle the window messages for this class.
     *
     * @param hwnd The window handle.
     * @param msg The type of message.
     * @param wParam The WPARAM. Varies per message.
     * @param lParam The LPARAM. Varies per message.
     * @return The resulting value.
     */
    LRESULT CALLBACK onWindowMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    /*
     * This static method is passed into the window class.
     *
     * @param hwnd The window handle.
     * @param msg The type of message.
     * @param wParam The WPARAM. Varies per message.
     * @param lParam The LPARAM. Varies per message.
     * @return The resulting value.
     */
    static LRESULT CALLBACK _onWindowMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

  public:
    /*
     * Gets the window handle.
     *
     * @return The window handle.
     */
    HWND getHandle();

    /*
     * Gets the width of the window.
     *
     * @return The window width.
     */
    uint32_t getWidth() const;

    /*
     * Gets the height of the window.
     *
     * @return The window height.
     */
    uint32_t getHeight() const;

    float getAspectRatio() const;

    /*
     * Signals that the window should close.
     */
    void setShouldClose();

    /*
     * Returns whether or not the window should close.
     *
     * @return Whether or not the window should close.
     */
    bool shouldClose() const;

    /*
     * Returns whether or not the window should resize.
     *
     * @return Whether or not the window should resize.
     */
    bool shouldResize() const;

  private:
    Window() = default;

    uint32_t m_width;
    uint32_t m_height;
    HWND     m_window      = nullptr;
    ATOM     m_windowClass = 0;

    bool m_shouldClose     = false;
    bool m_shouldResize    = false;
    bool m_fullscreenState = false;
};

} // namespace bisky::gfx