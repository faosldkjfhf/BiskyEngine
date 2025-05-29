#pragma once

#include "Common.h"

namespace D3D12
{

struct IWindowCallbacks
{
  virtual void OnKeyDown(WPARAM key) = 0;
  virtual void OnLeftMouseDown(WPARAM button, int x, int y) = 0;
  virtual void OnLeftMouseUp() = 0;
  virtual void OnMouseMove(WPARAM button, int x, int y) = 0;
};

} // namespace D3D12
