#pragma once

#include <DirectXMath.h>

using namespace DirectX;

namespace bisky::scene
{

/*
 * An arcball camera based on Ken Shoemake 1992.
 */
class ArcballCamera
{

  public:
    explicit ArcballCamera(uint32_t width, uint32_t height);
    ~ArcballCamera() = default;

    ArcballCamera(const ArcballCamera &)                   = delete;
    const ArcballCamera &operator=(const ArcballCamera &)  = delete;
    ArcballCamera(const ArcballCamera &&)                  = delete;
    const ArcballCamera &operator=(const ArcballCamera &&) = delete;

  public:
    void resize(uint32_t width, uint32_t height);
    void update();
    void onMove(int x, int y);
    void onBegin(int x, int y);
    void onEnd();

  private:
    XMVECTOR screenToVector(float sx, float sy);
    XMVECTOR quatFromBall(FXMVECTOR from, FXMVECTOR to);

  public:
    XMMATRIX getView() const;
    XMMATRIX getInverseView() const;
    XMMATRIX getProjection() const;
    XMVECTOR getRight() const;
    XMVECTOR getUp() const;
    XMVECTOR getForward() const;
    XMVECTOR getPosition() const;

  private:
    XMFLOAT3   m_cameraFocus;
    XMFLOAT3   m_lastCameraPosition;
    XMFLOAT4   m_cameraRotation;
    XMFLOAT4   m_viewRotation;
    float      m_zoom;
    float      m_distance;
    XMFLOAT4X4 m_view;
    XMFLOAT4X4 m_projection;
    XMFLOAT4X4 m_inverseView;

    uint32_t m_width;
    uint32_t m_height;
    float    m_fov;
    float    m_near;
    float    m_far;

    bool     m_drag;
    XMFLOAT4 m_qNow;
    XMFLOAT4 m_qDown;
    XMFLOAT3 m_downPoint;
};

} // namespace bisky::scene