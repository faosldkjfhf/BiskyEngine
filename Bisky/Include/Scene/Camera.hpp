#pragma once

#include <DirectXMath.h>

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
    DirectX::XMMATRIX   getView();
    DirectX::XMFLOAT4X4 getView4x4f();
    DirectX::XMMATRIX   getProjection() const;
    DirectX::XMFLOAT4X4 getProjection4x4f() const;
    DirectX::XMVECTOR   getPosition() const;
    DirectX::XMFLOAT3   getPosition3f() const;
    DirectX::XMVECTOR   getForward() const;
    DirectX::XMFLOAT3   getForward3f() const;
    DirectX::XMVECTOR   getUp() const;
    DirectX::XMFLOAT3   getUp3f() const;
    DirectX::XMVECTOR   getRight() const;
    DirectX::XMFLOAT3   getRight3f() const;
    bool                getDirty() const;

  protected:
    DirectX::XMFLOAT3   m_position;
    DirectX::XMFLOAT3   m_right;
    DirectX::XMFLOAT3   m_up;
    DirectX::XMFLOAT3   m_forward;
    DirectX::XMFLOAT4X4 m_view;
    DirectX::XMFLOAT4X4 m_projection;
    float               m_aspectRatio;
    float               m_fov       = 90.0f;
    float               m_near      = 0.1f;
    float               m_far       = 100.0f;
    float               m_speed     = 10.0f;
    bool                m_viewDirty = true;
};

} // namespace bisky::scene