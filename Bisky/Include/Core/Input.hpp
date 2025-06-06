#pragma once

#include <Windows.h>

namespace bisky::core
{

struct Input
{
    virtual void OnMouseMove(WPARAM key, int x, int y)     = 0;
    virtual void OnLeftMouseDown(WPARAM key, int x, int y) = 0;
    virtual void OnLeftMouseUp()                           = 0;
    virtual void OnKeyDown(WPARAM key)                     = 0;
};

} // namespace bisky::core