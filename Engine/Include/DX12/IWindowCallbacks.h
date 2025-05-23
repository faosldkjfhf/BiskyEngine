#pragma once

#include "Common.h"

namespace DX12
{

struct IWindowCallbacks
{
  virtual void OnKeyDown(WPARAM key) = 0;
  virtual void OnLeftMouseDown(WPARAM button, int x, int y) = 0;
  virtual void OnLeftMouseUp() = 0;
  virtual void OnMouseMove(WPARAM button, int x, int y) = 0;
};

} // namespace DX12
