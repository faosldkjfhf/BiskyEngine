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
    virtual ~Camera();

    Camera(const Camera &)                    = delete;
    const Camera &operator=(const Camera &)   = delete;
    Camera(const Camera &&)                   = delete;
    const Camera &&operator=(const Camera &&) = delete;

  protected:
    explicit Camera() = default;

  public:
    virtual bool input(const core::GameTimer *const timer);
    virtual void updateViewMatrix();
    void         reset();
    void         setPosition(float x, float y, float z);
    void         setLens(float aspectRatio, float nearZ, float farZ);
    void         setDirty();

  public:
    dx::XMMATRIX   getView();
    dx::XMFLOAT4X4 getView4x4f();
    dx::XMMATRIX   getProjection() const;
    dx::XMFLOAT4X4 getProjection4x4f() const;
    dx::XMVECTOR   getPosition() const;
    dx::XMFLOAT3   getPosition3f() const;
    dx::XMVECTOR   getForward() const;
    dx::XMFLOAT3   getForward3f() const;
    dx::XMVECTOR   getUp() const;
    dx::XMFLOAT3   getUp3f() const;
    dx::XMVECTOR   getRight() const;
    dx::XMFLOAT3   getRight3f() const;
    bool           getDirty() const;

  protected:
    dx::XMFLOAT3   m_position;
    dx::XMFLOAT3   m_right;
    dx::XMFLOAT3   m_up;
    dx::XMFLOAT3   m_forward;
    dx::XMFLOAT4X4 m_view;
    dx::XMFLOAT4X4 m_projection;
    float          m_aspectRatio;
    float          m_fov       = 90.0f;
    float          m_near      = 0.1f;
    float          m_far       = 100.0f;
    float          m_speed     = 10.0f;
    bool           m_viewDirty = true;
};

} // namespace bisky::scene