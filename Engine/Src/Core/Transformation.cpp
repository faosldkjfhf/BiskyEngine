#include "Common.h"

#include "Core/Transformation.h"

namespace Core
{

Transformation::Transformation()
{
  Reset();
}

void Transformation::Reset()
{
  XMStoreFloat4x4(&mRotation, XMMatrixIdentity());
  XMStoreFloat4x4(&mScale, XMMatrixIdentity());
  XMStoreFloat4x4(&mTranslation, XMMatrixIdentity());
}

void Transformation::SetScale(float x, float y, float z)
{
  XMStoreFloat4x4(&mScale, XMMatrixScaling(x, y, z));
}

void Transformation::AddTranslation(float x, float y, float z)
{
  XMStoreFloat4x4(&mTranslation, XMMatrixMultiply(XMMatrixTranslation(x, y, z), Translation()));
}

void Transformation::SetTranslation(float x, float y, float z)
{
  XMStoreFloat4x4(&mTranslation, XMMatrixTranslation(x, y, z));
}

void Transformation::AddRotation(XMMATRIX rotation)
{
  XMStoreFloat4x4(&mRotation, XMMatrixMultiply(XMLoadFloat4x4(&mRotation), rotation));
}

} // namespace Core