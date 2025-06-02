#include "Common.hpp"

#include "Graphics/Transform.hpp"

namespace bisky::gfx
{

Transform::Transform()
{
    XMStoreFloat3(&m_scale, math::FXMVECTOR{1.0f, 1.0f, 1.0f});
    XMStoreFloat3(&m_rotation, math::FXMVECTOR{0.0f, 0.0f, 0.0f});
    XMStoreFloat3(&m_translation, math::FXMVECTOR{0.0f, 0.0f, 0.0f});
}

void Transform::setScale(float x, float y, float z)
{
    XMStoreFloat3(&m_scale, math::FXMVECTOR{x, y, z});
}

void Transform::setTranslation(float x, float y, float z)
{
    XMStoreFloat3(&m_translation, math::FXMVECTOR{x, y, z});
}

void Transform::addTranslation(float x, float y, float z)
{
    XMStoreFloat3(&m_translation, math::XMVectorAdd(XMLoadFloat3(&m_translation), math::FXMVECTOR{x, y, z}));
}

void Transform::setRotation(float x, float y, float z)
{
    XMStoreFloat3(&m_rotation, math::FXMVECTOR{x, y, z});
}

math::XMMATRIX Transform::getLocalToWorld() const
{
    math::XMMATRIX s = math::XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
    math::XMMATRIX r = math::XMMatrixRotationQuaternion(math::XMQuaternionRotationRollPitchYaw(
        math::XMConvertToRadians(m_rotation.x), math::XMConvertToRadians(m_rotation.y),
        math::XMConvertToRadians(m_rotation.z)));
    math::XMMATRIX t = math::XMMatrixTranslation(m_translation.x, m_translation.y, m_translation.z);

    return s * r * t;
}

math::XMVECTOR Transform::getScale() const
{
    return XMLoadFloat3(&m_scale);
}

math::XMFLOAT3 Transform::getScale3f() const
{
    return m_scale;
}

math::XMMATRIX Transform::getScaleMatrix() const
{
    return math::XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
}

math::XMVECTOR Transform::getRotation() const
{
    return XMLoadFloat3(&m_rotation);
}

math::XMFLOAT3 Transform::getRotation3f() const
{
    return m_rotation;
}

math::XMMATRIX Transform::getRotationMatrix() const
{
    return math::XMMatrixRotationQuaternion(
        math::XMQuaternionRotationRollPitchYaw(m_rotation.y, m_rotation.x, m_rotation.z));
}

math::XMVECTOR Transform::getTranslation() const
{
    return XMLoadFloat3(&m_translation);
}

math::XMFLOAT3 Transform::getTranslation3f() const
{
    return m_translation;
}

math::XMMATRIX Transform::getTranslationMatrix() const
{
    return math::XMMatrixTranslation(m_translation.x, m_translation.y, m_translation.z);
}

} // namespace bisky::gfx