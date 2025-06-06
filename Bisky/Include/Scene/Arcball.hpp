#pragma once

namespace bisky::scene
{

/*
 * A Ken Shoemake Arcball Camera
 */
class Arcball
{
  public:
    explicit Arcball(float aspectRatio, float nearZ, float farZ);
    ~Arcball() = default;

    Arcball(const Arcball &)                    = delete;
    const Arcball &operator=(const Arcball &)   = delete;
    Arcball(const Arcball &&)                   = delete;
    const Arcball &&operator=(const Arcball &&) = delete;

  public:
    void setLens(float aspectRatio, float nearZ, float farZ);
    void rotate(dx::XMFLOAT2 start, dx::XMFLOAT2 end);

  private:
    void orthonormalizeCamera();
    void updateViewMatrix();

  public:
    dx::XMMATRIX   getView();
    dx::XMFLOAT4X4 getView4x4f();
    dx::XMMATRIX   getInverseView() const;
    dx::XMFLOAT4X4 getInverseView4x4f() const;
    dx::XMMATRIX   getProjection() const;
    dx::XMFLOAT4X4 getProjection4x4f() const;
    dx::XMVECTOR   getPosition() const;
    dx::XMFLOAT3   getPosition3f() const;

  private:
    dx::XMFLOAT3   m_focalPoint;
    dx::XMFLOAT3   m_position;
    dx::XMFLOAT3   m_up;
    dx::XMFLOAT3   m_right;
    dx::XMFLOAT4X4 m_centerTranslation;
    dx::XMFLOAT4   m_rotation;
    dx::XMFLOAT4X4 m_translation;
    dx::XMFLOAT4X4 m_view;
    dx::XMFLOAT4X4 m_inverseView;
    dx::XMFLOAT4X4 m_projection;

    float m_aspectRatio;
    float m_fov;
    float m_nearZ;
    float m_farZ;
};

} // namespace bisky::scene