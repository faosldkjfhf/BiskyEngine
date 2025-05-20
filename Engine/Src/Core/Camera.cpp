#include "Common.h"

#include "Core/Camera.h"
#include "DX12/Window.h"

namespace Core
{

Camera::Camera()
{
  mFov = XMConvertToRadians(90.0f);
  XMStoreFloat3(&mPosition, FXMVECTOR{0.0f, 0.0f, 3.0f});
  XMStoreFloat3(&mLookAt, XMVectorReplicate(0.0f));
  XMStoreFloat3(&mUp, FXMVECTOR{0.0f, 1.0f, 0.0f});
}

void Camera::SetPosition(FXMVECTOR position)
{
  XMStoreFloat3(&mPosition, position);
}

XMVECTOR Camera::Position() const
{
  return XMLoadFloat3(&mPosition);
}

XMMATRIX Camera::ViewMatrix() const
{
  auto position = XMLoadFloat3(&mPosition);
  auto lookAt = XMLoadFloat3(&mLookAt);
  auto up = XMLoadFloat3(&mUp);

  return XMMatrixLookAtLH(position, lookAt, up);
}

XMMATRIX Camera::ProjectionMatrix() const
{
  return XMMatrixPerspectiveFovLH(mFov, DX12::Window::Get().AspectRatio(), 0.1f, 1000.0f);
}

} // namespace Core