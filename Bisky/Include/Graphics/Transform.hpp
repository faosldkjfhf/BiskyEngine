#pragma once

#include "Common.hpp"

namespace bisky::gfx
{

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
    dx::XMMATRIX getLocalToWorld() const;
    dx::XMVECTOR getScale() const;
    dx::XMFLOAT3 getScale3f() const;
    dx::XMMATRIX getScaleMatrix() const;
    dx::XMVECTOR getRotation() const;
    dx::XMFLOAT3 getRotation3f() const;
    dx::XMMATRIX getRotationMatrix() const;
    dx::XMVECTOR getTranslation() const;
    dx::XMFLOAT3 getTranslation3f() const;
    dx::XMMATRIX getTranslationMatrix() const;

  private:
    dx::XMFLOAT3 m_scale;
    dx::XMFLOAT3 m_rotation;
    dx::XMFLOAT3 m_translation;
};

} // namespace bisky::gfx