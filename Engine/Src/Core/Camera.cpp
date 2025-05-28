#include "Common.h"

#include "Core/Camera.h"
#include "Core/Logger.h"
#include "DX12/Window.h"

namespace Core
{

Camera::Camera()
{
  XMStoreFloat4x4(&mProjectionMatrix,
                  XMMatrixPerspectiveFovLH(XMConvertToRadians(mFov), DX12::Window::Get().AspectRatio(), mNear, mFar));
  UpdateViewMatrix();
}

void Camera::SetPosition(FXMVECTOR position)
{
  XMStoreFloat3(&mPosition, position);
  UpdateViewMatrix();
}

void Camera::SetPosition(float x, float y, float z)
{
  XMStoreFloat3(&mPosition, FXMVECTOR{x, y, z});
  UpdateViewMatrix();
}

XMVECTOR Camera::Position() const
{
  return XMLoadFloat3(&mPosition);
}

XMFLOAT3 Camera::Position3f() const
{
  return mPosition;
}

XMVECTOR Camera::Right() const
{
  return XMLoadFloat3(&mRight);
}

XMFLOAT3 Camera::Right3f() const
{
  return mRight;
}

XMVECTOR Camera::Up() const
{
  return XMLoadFloat3(&mUp);
}

XMFLOAT3 Camera::Up3f() const
{
  return mUp;
}

XMVECTOR Camera::Look() const
{
  return XMLoadFloat3(&mLook);
}

XMFLOAT3 Camera::Look3f() const
{
  return mLook;
}

XMVECTOR Camera::ViewDirection() const
{
  return XMVector3Normalize(XMVectorSubtract(Look(), Position()));
}

XMFLOAT3 Camera::ViewDirection3f() const
{
  return mLook;
}

XMMATRIX Camera::ViewMatrix() const
{
  return XMLoadFloat4x4(&mViewMatrix);
}

XMFLOAT4X4 Camera::ViewMatrix4x4() const
{
  return mViewMatrix;
}

XMMATRIX Camera::ProjectionMatrix() const
{
  return XMLoadFloat4x4(&mProjectionMatrix);
}

XMFLOAT4X4 Camera::ProjectionMatrix4x4() const
{
  return mProjectionMatrix;
}

void Camera::UpdateViewMatrix()
{
  XMVECTOR r = XMLoadFloat3(&mRight);
  XMVECTOR u = XMLoadFloat3(&mUp);
  XMVECTOR l = XMLoadFloat3(&mLook);
  XMVECTOR p = XMLoadFloat3(&mPosition);

  // l = XMVector3Normalize(l);
  // u = XMVector3Normalize(XMVector3Cross(l, r));
  // r = XMVector3Cross(u, l);

  XMStoreFloat3(&mRight, r);
  XMStoreFloat3(&mLook, l);
  XMStoreFloat3(&mUp, u);

  XMStoreFloat4x4(&mViewMatrix, XMMatrixLookAtLH(p, l, u));
}

void Camera::SetLens(float fov, float aspectRatio, float nearZ, float farZ)
{
  mFov = fov;
  mNear = nearZ;
  mFar = farZ;

  XMStoreFloat4x4(&mProjectionMatrix, XMMatrixPerspectiveFovLH(XMConvertToRadians(fov), aspectRatio, nearZ, farZ));
}

Transformation &Camera::Transform()
{
  return mTransform;
}

} // namespace Core