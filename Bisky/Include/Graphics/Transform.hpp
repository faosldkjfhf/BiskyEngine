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
    math::XMMATRIX getLocalToWorld() const;
    math::XMVECTOR getScale() const;
    math::XMFLOAT3 getScale3f() const;
    math::XMMATRIX getScaleMatrix() const;
    math::XMVECTOR getRotation() const;
    math::XMFLOAT3 getRotation3f() const;
    math::XMMATRIX getRotationMatrix() const;
    math::XMVECTOR getTranslation() const;
    math::XMFLOAT3 getTranslation3f() const;
    math::XMMATRIX getTranslationMatrix() const;

  private:
    math::XMFLOAT3 m_scale;
    math::XMFLOAT3 m_rotation;
    math::XMFLOAT3 m_translation;
};

} // namespace bisky::gfx