#pragma once

#include <DirectXMath.h>

namespace bisky::gfx
{

/*
 * A basic set of transforms (scaling, rotation, translation).
 *
 * FIXME: Currently suffers from gimbal lock since using euler angles.
 */
class Transform
{
  public:
    explicit Transform();
    ~Transform() = default;

    Transform(const Transform &)                    = delete;
    const Transform &operator=(const Transform &)   = delete;
    Transform(const Transform &&)                   = delete;
    const Transform &&operator=(const Transform &&) = delete;

  public:
    void setScale(float x, float y, float z);
    void setTranslation(float x, float y, float z);
    void addTranslation(float x, float y, float z);
    void setRotation(float x, float y, float z);

  public:
    DirectX::XMMATRIX getLocalToWorld() const;
    DirectX::XMVECTOR getScale() const;
    DirectX::XMFLOAT3 getScale3f() const;
    DirectX::XMMATRIX getScaleMatrix() const;
    DirectX::XMVECTOR getRotation() const;
    DirectX::XMFLOAT3 getRotation3f() const;
    DirectX::XMMATRIX getRotationMatrix() const;
    DirectX::XMVECTOR getTranslation() const;
    DirectX::XMFLOAT3 getTranslation3f() const;
    DirectX::XMMATRIX getTranslationMatrix() const;

  private:
    DirectX::XMFLOAT3 m_scale;
    DirectX::XMFLOAT3 m_rotation;
    DirectX::XMFLOAT3 m_translation;
};

} // namespace bisky::gfx