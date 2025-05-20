#pragma once

#include "Core/Light.h"

namespace Core
{

struct ObjectConstants
{
  XMFLOAT4X4 World;
  XMFLOAT4X4 InverseWorld;
  XMFLOAT4X4 NormalMatrix;
};

struct MaterialConstants
{
  XMFLOAT3 Diffuse;
  float UseMaterial;
};

struct PassConstants
{
  XMFLOAT4X4 View;
  XMFLOAT4X4 Projection;
  XMFLOAT4 ViewPosition;
  Light Lights[1];
};

struct ShadowPassConstants
{
};

} // namespace Core
