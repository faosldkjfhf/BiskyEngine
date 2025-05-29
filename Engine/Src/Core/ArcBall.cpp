#include "Common.h"

#include "Core/ArcBall.h"
#include "Core/GlobalCamera.h"
#include "Core/MathHelpers.h"
#include "D3D12/Window.h"

namespace Core
{

ArcBall::ArcBall()
{
  XMStoreFloat2(&mStart, XMVectorReplicate(0.0f));
  XMStoreFloat2(&mEnd, XMVectorReplicate(0.0f));
  XMStoreFloat4x4(&mQuatRotation, XMMatrixIdentity());
}

void ArcBall::OnLeftMouseDown(int x, int y)
{
  // mStart = ConvertToNDC(x, y);
  XMStoreFloat2(&mStart, FXMVECTOR{(float)x, (float)y});
}

void ArcBall::OnMouseMove(int x, int y)
{
  // XMFLOAT4 position, pivot;
  // XMStoreFloat4(&position, FXMVECTOR{GlobalCamera::Get().Position3f().x, GlobalCamera::Get().Position3f().y,
  //                                    GlobalCamera::Get().Position3f().z, 1.0f});
  // XMStoreFloat4(&pivot, FXMVECTOR{GlobalCamera::Get().Look3f().x, GlobalCamera::Get().Look3f().y,
  //                                 GlobalCamera::Get().Look3f().z, 1.0f});

  // XMStoreFloat2(&mEnd, FXMVECTOR{(float)x, (float)y});

  // XMFLOAT2 dAngle, angle;
  // dAngle.x = 2 * XM_PI / D3D12::Window::Get().Width();
  // dAngle.y = XM_PI / D3D12::Window::Get().Height();
  // angle.x = (x - mStart.x) * dAngle.x;
  // angle.y = (y - mStart.y) * dAngle.y;

  // XMMATRIX qRotationX = XMMatrixRotationQuaternion(XMQuaternionRotationAxis(GlobalCamera::Get().Up(), angle.x));
  // XMStoreFloat4(&position, XMVectorAdd(XMVector4Transform(
  //                                          XMVectorSubtract(XMLoadFloat4(&position), XMLoadFloat4(&pivot)),
  //                                          qRotationX),
  //                                      XMLoadFloat4(&pivot)));
  // XMMATRIX qRotationY = XMMatrixRotationQuaternion(XMQuaternionRotationAxis(GlobalCamera::Get().Right(), angle.y));
  // XMStoreFloat4(&position, XMVectorAdd(XMVector4Transform(
  //                                          XMVectorSubtract(XMLoadFloat4(&position), XMLoadFloat4(&pivot)),
  //                                          qRotationY),
  //                                      XMLoadFloat4(&pivot)));

  // GlobalCamera::Get().SetPosition(position.x, position.y, position.z);
  // mStart = mEnd;

  // mEnd = ConvertToNDC(x, y);

  // XMFLOAT3 perp;
  // XMStoreFloat3(&perp, XMVector3Cross(XMLoadFloat3(&mStart), XMLoadFloat3(&mEnd)));
  // if (XMVectorGetX(XMVector3Length(XMLoadFloat3(&perp))) > 0.0f)
  //{
  //   XMStoreFloat4x4(&mQuatRotation, XMMatrixRotationQuaternion(ConvertToQuaternion(
  //                                       perp, XMVectorGetX(XMVector3Dot(XMLoadFloat3(&mStart),
  //                                       XMLoadFloat3(&mEnd))))));
  // }
  // else
  //{
  //   XMStoreFloat4x4(&mQuatRotation, XMMatrixIdentity());
  // }
}

XMFLOAT3 ArcBall::ConvertToNDC(int x, int y)
{
  XMFLOAT3 ndc{};
  float tx, ty;
  tx = (x * 1.0f / ((static_cast<float>(D3D12::Window::Get().Width() - 1.0f) * 0.5f))) - 1.0f;
  ty = (y * 1.0f / ((static_cast<float>(D3D12::Window::Get().Height() - 1.0f) * 0.5f))) - 1.0f;

  float quadrance = ((tx * tx) + (ty * ty));
  if (quadrance > 1.0f)
  {
    float norm = 1.0f / sqrtf(quadrance);
    XMStoreFloat3(&ndc, FXMVECTOR{tx * norm, ty * norm, 0.0f});
  }
  else
  {
    XMStoreFloat3(&ndc, FXMVECTOR{tx, ty, sqrtf(1.0f - quadrance)});
  }

  return ndc;
}

} // namespace Core