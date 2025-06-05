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

void Camera::reset()
{
    dx::XMStoreFloat3(&m_position, dx::FXMVECTOR{0.0f, 0.0f, 0.0f});
    dx::XMStoreFloat3(&m_forward, dx::FXMVECTOR{0.0f, 0.0f, 1.0f});
    dx::XMStoreFloat3(&m_right, dx::FXMVECTOR{1.0f, 0.0f, 0.0f});
    dx::XMStoreFloat3(&m_up, dx::FXMVECTOR{0.0f, 1.0f, 0.0f});
    m_viewDirty = true;

    updateViewMatrix();
}

void Camera::updateViewMatrix()
{
    if (m_viewDirty)
    {
        dx::XMVECTOR r = XMLoadFloat3(&m_right);
        dx::XMVECTOR u = XMLoadFloat3(&m_up);
        dx::XMVECTOR f = XMLoadFloat3(&m_forward);
        dx::XMVECTOR p = XMLoadFloat3(&m_position);

        f = dx::XMVector3Normalize(f);
        u = dx::XMVector3Normalize(dx::XMVector3Cross(f, r));
        r = dx::XMVector3Cross(u, f);

        XMStoreFloat4x4(&m_view, dx::XMMatrixLookAtLH(p, dx::XMVectorAdd(p, f), u));
        m_viewDirty = false;
    }
}

void Camera::setPosition(float x, float y, float z)
{
    XMStoreFloat3(&m_position, dx::FXMVECTOR{x, y, z});
    m_viewDirty = true;
}

void Camera::setLens(float aspectRatio, float nearZ, float farZ)
{
    m_aspectRatio = aspectRatio;
    m_near        = nearZ;
    m_far         = farZ;

    dx::XMStoreFloat4x4(
        &m_projection, dx::XMMatrixPerspectiveFovLH(dx::XMConvertToRadians(m_fov), aspectRatio, nearZ, farZ)
    );
    m_viewDirty = true;
}

void Camera::setDirty()
{
    m_viewDirty = true;
}

dx::XMMATRIX Camera::getView()
{
    return XMLoadFloat4x4(&m_view);
}

dx::XMFLOAT4X4 Camera::getView4x4f()
{
    return m_view;
}

dx::XMMATRIX Camera::getProjection() const
{
    return XMLoadFloat4x4(&m_projection);
}

dx::XMFLOAT4X4 Camera::getProjection4x4f() const
{
    return m_projection;
}

dx::XMVECTOR Camera::getPosition() const
{
    return dx::XMLoadFloat3(&m_position);
}

dx::XMFLOAT3 Camera::getPosition3f() const
{
    return m_position;
}

dx::XMVECTOR Camera::getForward() const
{
    return dx::XMLoadFloat3(&m_forward);
}

dx::XMFLOAT3 Camera::getForward3f() const
{
    return m_forward;
}

dx::XMVECTOR Camera::getUp() const
{
    return dx::XMLoadFloat3(&m_up);
}

dx::XMFLOAT3 Camera::getUp3f() const
{
    return m_up;
}

dx::XMVECTOR Camera::getRight() const
{
    return dx::XMLoadFloat3(&m_right);
}

dx::XMFLOAT3 Camera::getRight3f() const
{
    return m_right;
}

bool Camera::getDirty() const
{
    return m_viewDirty;
}

} // namespace bisky::scene