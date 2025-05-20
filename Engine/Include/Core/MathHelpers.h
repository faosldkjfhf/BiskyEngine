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

} // namespace Core