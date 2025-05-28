#pragma once

#include "Common.h"

namespace Core
{

class Transformation
{
public:
  Transformation();

  void Reset();
  void SetScale(float x, float y, float z);
  void AddTranslation(float x, float y, float z);
  void SetTranslation(float x, float y, float z);
  void AddRotation(XMMATRIX rotation);

  inline XMMATRIX Rotation() const
  {
    return XMLoadFloat4x4(&mRotation);
  }

  inline XMFLOAT4X4 Rotation4x4() const
  {
    return mRotation;
  }

  inline XMMATRIX Scale() const
  {
    return XMLoadFloat4x4(&mScale);
  }

  inline XMFLOAT4X4 Scale4x4() const
  {
    return mScale;
  }

  inline XMMATRIX Translation() const
  {
    return XMLoadFloat4x4(&mTranslation);
  }

  inline XMFLOAT4X4 Translation4x4() const
  {
    return mTranslation;
  }

  inline XMMATRIX LocalToWorldMatrix() const
  {
    return Scale() * Rotation() * Translation();
  }

  inline XMFLOAT4X4 LocalToWorldMatrix4x4() const
  {
    XMFLOAT4X4 mat;
    XMStoreFloat4x4(&mat, LocalToWorldMatrix());
    return mat;
  }

private:
  XMFLOAT4X4 mRotation;
  XMFLOAT4X4 mScale;
  XMFLOAT4X4 mTranslation;
};

} // namespace Core