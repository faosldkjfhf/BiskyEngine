#include "Common.hpp"

#include "Scene/ArcballCamera.hpp"

namespace bisky::scene
{

ArcballCamera::ArcballCamera(uint32_t width, uint32_t height)
    : m_width(width), m_height(height), m_fov(90.0f), m_near(0.1f), m_far(100.0f), m_zoom(1.0f), m_distance(5.0f),
      m_drag(false)
{
    resize(width, height);
    XMStoreFloat4(&m_cameraRotation, XMQuaternionIdentity());
    XMStoreFloat4(&m_qNow, XMQuaternionIdentity());
    XMStoreFloat4(&m_qDown, XMQuaternionIdentity());
    XMStoreFloat3(&m_cameraFocus, XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f));
    update();
}

void ArcballCamera::resize(uint32_t width, uint32_t height)
{
    m_width  = width;
    m_height = height;

    XMStoreFloat4x4(
        &m_projection,
        XMMatrixPerspectiveFovLH(XMConvertToRadians(m_fov), static_cast<float>(width) / height, m_near, m_far)
    );
}

void ArcballCamera::update()
{
    XMMATRIX quat = XMMatrixRotationQuaternion(XMLoadFloat4(&m_cameraRotation));
    XMVECTOR dir  = XMVector3Transform(XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f), quat);
    XMVECTOR up   = XMVector3Transform(XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), quat);
    XMStoreFloat3(
        &m_lastCameraPosition,
        XMVectorAdd(XMLoadFloat3(&m_cameraFocus), XMVectorMultiply(dir, XMVectorReplicate(-m_distance * m_zoom)))
    );
    XMStoreFloat4x4(&m_view, XMMatrixLookAtLH(XMLoadFloat3(&m_lastCameraPosition), XMLoadFloat3(&m_cameraFocus), up));
    XMStoreFloat4x4(&m_inverseView, XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_view)));
}

void ArcballCamera::onMove(int x, int y)
{
    if (m_drag)
    {
        XMVECTOR curr = screenToVector(float(x), float(y));

        XMStoreFloat4(
            &m_qNow, XMQuaternionNormalize(
                         XMQuaternionMultiply(XMLoadFloat4(&m_qDown), quatFromBall(XMLoadFloat3(&m_downPoint), curr))
                     )
        );

        XMStoreFloat4(&m_cameraRotation, XMQuaternionInverse(XMLoadFloat4(&m_qNow)));
        update();
    }
}

void ArcballCamera::onBegin(int x, int y)
{
    m_drag  = true;
    m_qDown = m_qNow;
    XMStoreFloat3(&m_downPoint, screenToVector(float(x), float(y)));
    LOG_INFO(core::float3(m_downPoint));
}

void ArcballCamera::onEnd()
{
    m_drag = false;
}

XMVECTOR ArcballCamera::screenToVector(float sx, float sy)
{
    float x   = -(sx - float(m_width) / 2.0f) / (float(m_width) / 2.0f);
    float y   = (sy - float(m_height) / 2.0f) / (float(m_height) / 2.0f);
    float z   = 0.0f;
    float dot = x * x + y * y;
    if (dot > 1.0f)
    {
        float s = 1.0f / sqrtf(dot);
        x *= s;
        y *= s;
    }
    else
    {
        z = sqrtf(1.0f - dot);
    }

    return XMVectorSet(x, y, z, 0.0f);
}

XMVECTOR ArcballCamera::quatFromBall(FXMVECTOR from, FXMVECTOR to)
{
    XMVECTOR w   = XMVector3Dot(from, to);
    XMVECTOR xyz = XMVector3Cross(from, to);
    return XMVectorSelect(w, xyz, g_XMSelect1110);
}

XMMATRIX ArcballCamera::getView() const
{
    return XMLoadFloat4x4(&m_view);
}

XMMATRIX ArcballCamera::getInverseView() const
{
    return XMLoadFloat4x4(&m_inverseView);
}

XMMATRIX ArcballCamera::getProjection() const
{
    return XMLoadFloat4x4(&m_projection);
}

XMVECTOR ArcballCamera::getRight() const
{
    return XMVector4Transform(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), XMLoadFloat4x4(&m_inverseView));
}

XMVECTOR ArcballCamera::getUp() const
{
    return XMVector4Transform(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XMLoadFloat4x4(&m_inverseView));
}

XMVECTOR ArcballCamera::getForward() const
{
    return XMVector4Transform(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), XMLoadFloat4x4(&m_inverseView));
}

XMVECTOR ArcballCamera::getPosition() const
{
    return XMVector4Transform(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), XMLoadFloat4x4(&m_inverseView));
}

} // namespace bisky::scene