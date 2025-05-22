#pragma once

#include "Common.h"

namespace Core
{

struct Vertex
{
  XMFLOAT3 Position;
  XMFLOAT3 Normal;
  XMFLOAT2 TexCoord;
  XMFLOAT3 Tangent;
};

} // namespace Core
