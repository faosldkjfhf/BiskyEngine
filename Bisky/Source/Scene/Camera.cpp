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
        updateViewMatrix();
        return true;
    }

    if (GetAsyncKeyState('S') & 0x8000)
    {
        m_position.z -= m_speed * timer->deltaTime();
        updateViewMatrix();
        return true;
    }

    if (GetAsyncKeyState('A') & 0x8000)
    {
        m_position.x -= m_speed * timer->deltaTime();
        updateViewMatrix();
        return true;
    }

    if (GetAsyncKeyState('D') & 0x8000)
    {
        m_position.x += m_speed * timer->deltaTime();
        updateViewMatrix();
        return true;
    }

    return false;
}

void Camera::reset()
{
    math::XMStoreFloat3(&m_position, math::FXMVECTOR{0.0f, 0.0f, -5.0f});
    math::XMStoreFloat3(&m_forward, math::FXMVECTOR{0.0f, 0.0f, 1.0f});
    math::XMStoreFloat3(&m_right, math::FXMVECTOR{1.0f, 0.0f, 0.0f});
    math::XMStoreFloat3(&m_up, math::FXMVECTOR{0.0f, 1.0f, 0.0f});

    updateViewMatrix();
}

void Camera::updateViewMatrix()
{
    math::XMVECTOR r = XMLoadFloat3(&m_right);
    math::XMVECTOR u = XMLoadFloat3(&m_up);
    math::XMVECTOR f = XMLoadFloat3(&m_forward);
    math::XMVECTOR p = XMLoadFloat3(&m_position);

    XMStoreFloat4x4(&m_view, math::XMMatrixLookAtLH(p, math::XMVectorAdd(p, f), u));
}

void Camera::setPosition(float x, float y, float z)
{
    XMStoreFloat3(&m_position, math::FXMVECTOR{x, y, z});
    updateViewMatrix();
}

void Camera::setLens(float aspectRatio, float nearZ, float farZ)
{
    m_aspectRatio = aspectRatio;
    m_near        = nearZ;
    m_far         = farZ;

    math::XMStoreFloat4x4(
        &m_projection, math::XMMatrixPerspectiveFovLH(math::XMConvertToRadians(m_fov), aspectRatio, nearZ, farZ)
    );
    updateViewMatrix();
}

math::XMMATRIX Camera::getView()
{
    return XMLoadFloat4x4(&m_view);
}

math::XMFLOAT4X4 Camera::getView4x4f()
{
    return m_view;
}

math::XMMATRIX Camera::getProjection() const
{
    return XMLoadFloat4x4(&m_projection);
}

math::XMFLOAT4X4 Camera::getProjection4x4f() const
{
    return m_projection;
}

math::XMVECTOR Camera::getPosition() const
{
    return math::XMLoadFloat3(&m_position);
}

math::XMFLOAT3 Camera::getPosition3f() const
{
    return m_position;
}

math::XMVECTOR Camera::getForward() const
{
    return math::XMLoadFloat3(&m_forward);
}

math::XMFLOAT3 Camera::getForward3f() const
{
    return m_forward;
}

math::XMVECTOR Camera::getUp() const
{
    return math::XMLoadFloat3(&m_up);
}

math::XMFLOAT3 Camera::getUp3f() const
{
    return m_up;
}

math::XMVECTOR Camera::getRight() const
{
    return math::XMLoadFloat3(&m_right);
}

math::XMFLOAT3 Camera::getRight3f() const
{
    return m_right;
}

} // namespace bisky::scene