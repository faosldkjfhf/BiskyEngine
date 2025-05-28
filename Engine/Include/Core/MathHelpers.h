#pragma once

#include "Common.h"

namespace Core
{

inline XMFLOAT4X4 MatrixIdentity()
{
  XMFLOAT4X4 ret;
  XMStoreFloat4x4(&ret, XMMatrixIdentity());
  return ret;
}

inline XMVECTOR ConvertToQuaternion(XMFLOAT3 axis, float angle)
{
  float halfAngle = angle * 0.5f;
  return FXMVECTOR{cosf(halfAngle), axis.x * sinf(halfAngle), axis.y * sinf(halfAngle), axis.z * sinf(halfAngle)};
}

} // namespace Core