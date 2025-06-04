#pragma once

#include "Common.hpp"
#include "Graphics/Device.hpp"

namespace bisky::core
{
class GameTimer;
}

namespace bisky::scene
{

class Camera
{
  public:
    explicit Camera(float aspectRatio, float nearZ, float farZ);
    ~Camera();

    Camera(const Camera &)                    = delete;
    const Camera &operator=(const Camera &)   = delete;
    Camera(const Camera &&)                   = delete;
    const Camera &&operator=(const Camera &&) = delete;

  public:
    virtual bool input(const core::GameTimer *const timer);
    void         reset();
    void         updateViewMatrix();
    void         setPosition(float x, float y, float z);
    void         setLens(float aspectRatio, float nearZ, float farZ);
    void         setDirty();

  public:
    math::XMMATRIX   getView();
    math::XMFLOAT4X4 getView4x4f();
    math::XMMATRIX   getProjection() const;
    math::XMFLOAT4X4 getProjection4x4f() const;
    math::XMVECTOR   getPosition() const;
    math::XMFLOAT3   getPosition3f() const;
    math::XMVECTOR   getForward() const;
    math::XMFLOAT3   getForward3f() const;
    math::XMVECTOR   getUp() const;
    math::XMFLOAT3   getUp3f() const;
    math::XMVECTOR   getRight() const;
    math::XMFLOAT3   getRight3f() const;
    bool             getDirty() const;

  private:
    math::XMFLOAT3   m_position;
    math::XMFLOAT3   m_right;
    math::XMFLOAT3   m_up;
    math::XMFLOAT3   m_forward;
    math::XMFLOAT4X4 m_view;
    math::XMFLOAT4X4 m_projection;
    float            m_aspectRatio;
    float            m_fov       = 90.0f;
    float            m_near      = 0.1f;
    float            m_far       = 100.0f;
    float            m_speed     = 10.0f;
    bool             m_viewDirty = true;
};

} // namespace bisky::scene