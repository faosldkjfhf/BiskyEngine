#pragma once

#include "Common.h"
#include "Core/Transformation.h"

namespace Core
{
class Camera
{
public:
  Camera();

  void SetPosition(FXMVECTOR position);
  void SetPosition(float x, float y, float z);

  XMVECTOR Position() const;
  XMFLOAT3 Position3f() const;

  XMVECTOR Right() const;
  XMFLOAT3 Right3f() const;

  XMVECTOR Up() const;
  XMFLOAT3 Up3f() const;
  XMVECTOR Look() const;
  XMFLOAT3 Look3f() const;
  XMVECTOR ViewDirection() const;
  XMFLOAT3 ViewDirection3f() const;

  XMMATRIX ViewMatrix() const;
  XMFLOAT4X4 ViewMatrix4x4() const;
  XMMATRIX ProjectionMatrix() const;
  XMFLOAT4X4 ProjectionMatrix4x4() const;

  void UpdateViewMatrix();
  void SetLens(float fov, float aspectRatio, float nearZ, float farZ);

  Transformation &Transform();

protected:
  XMFLOAT3 mPosition = {0.0f, 0.0f, -3.0f};
  XMFLOAT3 mLook = {0.0f, 0.0f, 0.0f};
  XMFLOAT3 mUp = {0.0f, 1.0f, 0.0f};
  XMFLOAT3 mRight = {1.0f, 0.0f, 0.0f};
  float mNear = 0.1f;
  float mFar = 100.0f;
  float mFov = 90.0f;
  XMFLOAT4X4 mViewMatrix;
  XMFLOAT4X4 mProjectionMatrix;
  Transformation mTransform;

private:
};
} // namespace Core