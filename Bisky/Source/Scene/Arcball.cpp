#include "Common.hpp"

#include "Scene/Arcball.hpp"

namespace bisky::scene
{

static dx::XMVECTOR ndcToArcBall(dx::FXMVECTOR ndc);

Arcball::Arcball(float aspectRatio, float nearZ, float farZ)
    : m_aspectRatio(aspectRatio), m_nearZ(nearZ), m_farZ(farZ), m_fov(90.0f)
{
    XMStoreFloat3(&m_focalPoint, dx::FXMVECTOR{0.0f, 0.0f, 0.0f});
    XMStoreFloat3(&m_position, dx::FXMVECTOR{0.0f, 0.0f, -5.0f});
    XMStoreFloat3(&m_up, dx::FXMVECTOR{0.0f, 1.0f, 0.0f});
    XMStoreFloat3(&m_right, dx::FXMVECTOR{1.0f, 0.0f, 0.0f});

    orthonormalizeCamera();
    updateViewMatrix();
}

void Arcball::setLens(float aspectRatio, float nearZ, float farZ)
{
    m_aspectRatio = aspectRatio;
    m_nearZ       = nearZ;
    m_farZ        = farZ;

    XMStoreFloat4x4(
        &m_projection, dx::XMMatrixPerspectiveFovLH(dx::XMConvertToRadians(m_fov), aspectRatio, nearZ, farZ)
    );
}

void Arcball::rotate(dx::XMFLOAT2 start, dx::XMFLOAT2 end)
{
    // update rotation matrix based on dx and dy
    auto lastMouse = ndcToArcBall(XMLoadFloat2(&start));
    auto currMouse = ndcToArcBall(XMLoadFloat2(&end));

    dx::XMFLOAT4 quat;
    XMStoreFloat4(&quat, dx::XMVector3Cross(XMLoadFloat2(&start), XMLoadFloat2(&end)));
    quat.w = dx::XMVectorGetX(dx::XMVector3Dot(XMLoadFloat2(&start), XMLoadFloat2(&end)));

    // TODO: figure out what's wrong with the math
    XMStoreFloat4(&m_rotation, dx::XMQuaternionMultiply(XMLoadFloat4(&quat), XMLoadFloat4(&m_rotation)));

    // update view matrix
    updateViewMatrix();
}

void Arcball::orthonormalizeCamera()
{
    dx::XMVECTOR u = XMLoadFloat3(&m_up);
    dx::XMVECTOR r = XMLoadFloat3(&m_right);
    dx::XMVECTOR p = XMLoadFloat3(&m_position);
    dx::XMVECTOR f = XMLoadFloat3(&m_focalPoint);

    /*
     * Left hand rule
     * 1. index finger in direction of a
     * 2. middle finger in direction of b
     * 3. thumb is resulting cross(a, b)
     */
    dx::XMVECTOR forward = dx::XMVectorSubtract(f, p);
    dx::XMVECTOR z       = dx::XMVector3Normalize(forward);
    dx::XMVECTOR x       = dx::XMVector3Normalize(dx::XMVector3Cross(u, z));
    dx::XMVECTOR y       = dx::XMVector3Cross(z, x);

    // transforms our coordinates to be relative to our focal point
    XMStoreFloat4x4(&m_centerTranslation, dx::XMMatrixInverse(nullptr, dx::XMMatrixTranslationFromVector(f)));

    dx::XMMATRIX mat = {};
    mat.r[0]         = x;
    mat.r[1]         = y;
    mat.r[2]         = z;

    // create rotation matrix
    XMStoreFloat4(&m_rotation, dx::XMQuaternionRotationMatrix((mat)));

    // final translation applied after the rotation
    XMStoreFloat4x4(
        &m_translation, dx::XMMatrixTranslation(0.0f, 0.0f, dx::XMVectorGetX(dx::XMVector3Length(forward)))
    );
}

void Arcball::updateViewMatrix()
{
    XMStoreFloat4x4(
        &m_view, dx::XMLoadFloat4x4(&m_translation) * dx::XMMatrixRotationQuaternion(dx::XMLoadFloat4(&m_rotation)) *
                     dx::XMLoadFloat4x4(&m_centerTranslation)
    );
    XMStoreFloat4x4(&m_inverseView, dx::XMMatrixInverse(nullptr, dx::XMLoadFloat4x4(&m_view)));
}

dx::XMMATRIX Arcball::getView()
{
    return dx::XMLoadFloat4x4(&m_view);
}

dx::XMFLOAT4X4 Arcball::getView4x4f()
{
    return m_view;
}

dx::XMMATRIX Arcball::getInverseView() const
{
    return XMLoadFloat4x4(&m_inverseView);
}

dx::XMFLOAT4X4 Arcball::getInverseView4x4f() const
{
    return m_inverseView;
}

dx::XMMATRIX Arcball::getProjection() const
{
    return dx::XMLoadFloat4x4(&m_projection);
}

dx::XMFLOAT4X4 Arcball::getProjection4x4f() const
{
    return m_projection;
}

dx::XMVECTOR Arcball::getPosition() const
{
    return XMLoadFloat3(&m_position);
}

dx::XMFLOAT3 Arcball::getPosition3f() const
{
    return m_position;
}

static dx::XMVECTOR ndcToArcBall(dx::FXMVECTOR ndc)
{
    const float dist = dx::XMVectorGetX(dx::XMVector2Dot(ndc, ndc));
    if (dist <= 1.0f)
    {
        float s = 1.0f / std::sqrtf(dist);
        return dx::FXMVECTOR{dx::XMVectorGetX(ndc), dx::XMVectorGetY(ndc), 0.0f, std::sqrtf(1.0f - dist)};
    }
    else
    {
        dx::XMVECTOR proj = dx::XMVector2Normalize(ndc);
        return dx::FXMVECTOR{dx::XMVectorGetX(proj), dx::XMVectorGetY(proj), 0.0f, 0.0f};
    }
}

} // namespace bisky::scene