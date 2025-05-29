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
  float Metallic;
  float Roughness;
  float AmbientOcclusion;
  bool UseMaterial;
  float Buffer1;
};

struct PassConstants
{
  XMFLOAT4X4 View;
  XMFLOAT4X4 Projection;
  XMFLOAT4 ViewPosition;
  PointLight PointLights[1];
  DirectionalLight DirectionalLights[1];
};

struct ShadowPassConstants
{
};

} // namespace Core
