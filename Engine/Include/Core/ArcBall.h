#pragma once

#include "Common.h"

namespace Core
{

class ArcBall
{
public:
  ArcBall();

  void OnLeftMouseDown(int x, int y);
  void OnMouseMove(int x, int y);

  inline XMMATRIX Rotation() const
  {
    return XMLoadFloat4x4(&mQuatRotation);
  }

  inline XMFLOAT4X4 Rotation4x4() const
  {
    return mQuatRotation;
  }

private:
  XMFLOAT3 ConvertToNDC(int x, int y);

  XMFLOAT2 mStart;
  XMFLOAT2 mEnd;
  // XMFLOAT3 mStart;
  // XMFLOAT3 mEnd;
  XMFLOAT4X4 mQuatRotation;
};

} // namespace Core