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
  XMFLOAT3 Bitangent;
};

} // namespace Core
