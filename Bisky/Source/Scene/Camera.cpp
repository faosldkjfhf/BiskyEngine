#include "Common.hpp"

#include "Core/GameTimer.hpp"
#include "Scene/Camera.hpp"

namespace bisky::scene
{

Camera::Camera(float aspectRatio, float nearZ, float farZ)
{
    reset();
    updateViewMatrix();
    setLens(aspectRatio, nearZ, farZ);
}

Camera::~Camera()
{
}

bool Camera::input(const core::GameTimer *const timer)
{
    if (GetAsyncKeyState('W') & 0x8000)
    {
        m_position.z += m_speed * timer->deltaTime();
        m_viewDirty = true;
    }

    if (GetAsyncKeyState('S') & 0x8000)
    {
        m_position.z -= m_speed * timer->deltaTime();
        m_viewDirty = true;
    }

    if (GetAsyncKeyState('A') & 0x8000)
    {
        m_position.x -= m_speed * timer->deltaTime();
        m_viewDirty = true;
    }

    if (GetAsyncKeyState('D') & 0x8000)
    {
        m_position.x += m_speed * timer->deltaTime();
        m_viewDirty = true;
    }

    return m_viewDirty;
}

void Camera::updateViewMatrix()
{
    if (m_viewDirty)
    {
        XMVECTOR r = XMLoadFloat3(&m_right);
        XMVECTOR u = XMLoadFloat3(&m_up);
        XMVECTOR f = XMLoadFloat3(&m_forward);
        XMVECTOR p = XMLoadFloat3(&m_position);

        f = XMVector3Normalize(f);
        u = XMVector3Normalize(XMVector3Cross(f, r));
        r = XMVector3Cross(u, f);

        XMStoreFloat4x4(&m_view, XMMatrixLookAtLH(p, XMVectorAdd(p, f), u));
        m_viewDirty = false;
    }
}

void Camera::reset()
{
    XMStoreFloat3(&m_position, FXMVECTOR{0.0f, 0.0f, 0.0f});
    XMStoreFloat3(&m_forward, FXMVECTOR{0.0f, 0.0f, 1.0f});
    XMStoreFloat3(&m_right, FXMVECTOR{1.0f, 0.0f, 0.0f});
    XMStoreFloat3(&m_up, FXMVECTOR{0.0f, 1.0f, 0.0f});
    m_viewDirty = true;

    updateViewMatrix();
}

void Camera::setPosition(float x, float y, float z)
{
    XMStoreFloat3(&m_position, FXMVECTOR{x, y, z});
    m_viewDirty = true;
}

void Camera::setLens(float aspectRatio, float nearZ, float farZ)
{
    m_aspectRatio = aspectRatio;
    m_near        = nearZ;
    m_far         = farZ;

    XMStoreFloat4x4(
        &m_projection, XMMatrixPerspectiveFovLH(XMConvertToRadians(m_fov), aspectRatio, nearZ, farZ)
    );
    m_viewDirty = true;
}

void Camera::setDirty()
{
    m_viewDirty = true;
}

XMMATRIX Camera::getView()
{
    return XMLoadFloat4x4(&m_view);
}

XMFLOAT4X4 Camera::getView4x4f()
{
    return m_view;
}

XMMATRIX Camera::getProjection() const
{
    return XMLoadFloat4x4(&m_projection);
}

XMFLOAT4X4 Camera::getProjection4x4f() const
{
    return m_projection;
}

XMVECTOR Camera::getPosition() const
{
    return XMLoadFloat3(&m_position);
}

XMFLOAT3 Camera::getPosition3f() const
{
    return m_position;
}

XMVECTOR Camera::getForward() const
{
    return XMLoadFloat3(&m_forward);
}

XMFLOAT3 Camera::getForward3f() const
{
    return m_forward;
}

XMVECTOR Camera::getUp() const
{
    return XMLoadFloat3(&m_up);
}

XMFLOAT3 Camera::getUp3f() const
{
    return m_up;
}

XMVECTOR Camera::getRight() const
{
    return XMLoadFloat3(&m_right);
}

XMFLOAT3 Camera::getRight3f() const
{
    return m_right;
}

bool Camera::getDirty() const
{
    return m_viewDirty;
}

} // namespace bisky::scene