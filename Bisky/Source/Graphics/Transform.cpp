#include "Common.hpp"

#include "Graphics/Transform.hpp"

namespace bisky::gfx
{

Transform::Transform()
{
    XMStoreFloat3(&m_scale, FXMVECTOR{1.0f, 1.0f, 1.0f});
    XMStoreFloat3(&m_rotation, FXMVECTOR{0.0f, 0.0f, 0.0f});
    XMStoreFloat3(&m_translation, FXMVECTOR{0.0f, 0.0f, 0.0f});
    XMStoreFloat4(&m_rotationQuat, DirectX::XMQuaternionIdentity());
    LOG_INFO(core::float4(m_rotationQuat));
}

void Transform::setScale(float x, float y, float z)
{
    XMStoreFloat3(&m_scale, FXMVECTOR{x, y, z});
}

void Transform::setTranslation(float x, float y, float z)
{
    XMStoreFloat3(&m_translation, FXMVECTOR{x, y, z});
}

void Transform::addTranslation(float x, float y, float z)
{
    XMStoreFloat3(&m_translation, XMVectorAdd(XMLoadFloat3(&m_translation), FXMVECTOR{x, y, z}));
}

void Transform::setRotation(float x, float y, float z)
{
    XMStoreFloat3(&m_rotation, FXMVECTOR{x, y, z});
}

XMMATRIX Transform::getLocalToWorld() const
{
    XMMATRIX s = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
    XMMATRIX r = XMMatrixRotationQuaternion(XMQuaternionRotationRollPitchYaw(
        XMConvertToRadians(m_rotation.x), XMConvertToRadians(m_rotation.y), XMConvertToRadians(m_rotation.z)
    ));
    XMMATRIX t = XMMatrixTranslation(m_translation.x, m_translation.y, m_translation.z);

    return s * r * t;
}

XMVECTOR Transform::getScale() const
{
    return XMLoadFloat3(&m_scale);
}

XMFLOAT3 Transform::getScale3f() const
{
    return m_scale;
}

XMMATRIX Transform::getScaleMatrix() const
{
    return XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
}

XMVECTOR Transform::getRotation() const
{
    return XMLoadFloat3(&m_rotation);
}

XMFLOAT3 Transform::getRotation3f() const
{
    return m_rotation;
}

XMMATRIX Transform::getRotationMatrix() const
{
    return XMMatrixRotationQuaternion(XMQuaternionRotationRollPitchYaw(m_rotation.y, m_rotation.x, m_rotation.z));
}

XMVECTOR Transform::getTranslation() const
{
    return XMLoadFloat3(&m_translation);
}

XMFLOAT3 Transform::getTranslation3f() const
{
    return m_translation;
}

XMMATRIX Transform::getTranslationMatrix() const
{
    return XMMatrixTranslation(m_translation.x, m_translation.y, m_translation.z);
}

} // namespace bisky::gfx