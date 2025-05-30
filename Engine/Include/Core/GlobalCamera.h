#pragma once

#include "Camera.h"

namespace Core
{

class GlobalCamera : public Camera
{
public:
  void UpdatePosition(int x, int y);

  GlobalCamera(const GlobalCamera &) = delete;
  const GlobalCamera &operator=(const GlobalCamera &) = delete;
  inline static GlobalCamera &Get()
  {
    static GlobalCamera instance;
    return instance;
  }

private:
  GlobalCamera() = default;
};

} // namespace Core
