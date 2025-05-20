#pragma once

#include "Common.h"

namespace Core
{
class Camera
{
public:
  Camera();

  void SetPosition(FXMVECTOR position);
  XMVECTOR Position() const;

  XMMATRIX ViewMatrix() const;
  XMMATRIX ProjectionMatrix() const;

protected:
  XMFLOAT3 mPosition;
  XMFLOAT3 mLookAt;
  XMFLOAT3 mUp;
  float mFov;

private:
};
} // namespace Core