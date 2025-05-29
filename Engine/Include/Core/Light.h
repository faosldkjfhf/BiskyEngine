#pragma once

#include "Common.h"

struct PointLight
{
  XMFLOAT4 Position;
  XMFLOAT4 Strength;
};

struct DirectionalLight
{
  XMFLOAT4 Direction;
  XMFLOAT4 Strength;
};
