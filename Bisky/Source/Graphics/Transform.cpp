#include "Common.hpp"

#include "Graphics/Transform.hpp"

namespace bisky::gfx
{

Transform::Transform()
{
    XMStoreFloat3(&m_scale, dx::FXMVECTOR{1.0f, 1.0f, 1.0f});
    XMStoreFloat3(&m_rotation, dx::FXMVECTOR{0.0f, 0.0f, 0.0f});
    XMStoreFloat3(&m_translation, dx::FXMVECTOR{0.0f, 0.0f, 0.0f});
}

void Transform::setScale(float x, float y, float z)
{
    XMStoreFloat3(&m_scale, dx::FXMVECTOR{x, y, z});
}

void Transform::setTranslation(float x, float y, float z)
{
    XMStoreFloat3(&m_translation, dx::FXMVECTOR{x, y, z});
}

void Transform::addTranslation(float x, float y, float z)
{
    XMStoreFloat3(&m_translation, dx::XMVectorAdd(XMLoadFloat3(&m_translation), dx::FXMVECTOR{x, y, z}));
}

void Transform::setRotation(float x, float y, float z)
{
    XMStoreFloat3(&m_rotation, dx::FXMVECTOR{x, y, z});
}

dx::XMMATRIX Transform::getLocalToWorld() const
{
    dx::XMMATRIX s = dx::XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
    dx::XMMATRIX r = dx::XMMatrixRotationQuaternion(dx::XMQuaternionRotationRollPitchYaw(
        dx::XMConvertToRadians(m_rotation.x), dx::XMConvertToRadians(m_rotation.y),
        dx::XMConvertToRadians(m_rotation.z)));
    dx::XMMATRIX t = dx::XMMatrixTranslation(m_translation.x, m_translation.y, m_translation.z);

    return s * r * t;
}

dx::XMVECTOR Transform::getScale() const
{
    return XMLoadFloat3(&m_scale);
}

dx::XMFLOAT3 Transform::getScale3f() const
{
    return m_scale;
}

dx::XMMATRIX Transform::getScaleMatrix() const
{
    return dx::XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
}

dx::XMVECTOR Transform::getRotation() const
{
    return XMLoadFloat3(&m_rotation);
}

dx::XMFLOAT3 Transform::getRotation3f() const
{
    return m_rotation;
}

dx::XMMATRIX Transform::getRotationMatrix() const
{
    return dx::XMMatrixRotationQuaternion(
        dx::XMQuaternionRotationRollPitchYaw(m_rotation.y, m_rotation.x, m_rotation.z));
}

dx::XMVECTOR Transform::getTranslation() const
{
    return XMLoadFloat3(&m_translation);
}

dx::XMFLOAT3 Transform::getTranslation3f() const
{
    return m_translation;
}

dx::XMMATRIX Transform::getTranslationMatrix() const
{
    return dx::XMMatrixTranslation(m_translation.x, m_translation.y, m_translation.z);
}

} // namespace bisky::gfx