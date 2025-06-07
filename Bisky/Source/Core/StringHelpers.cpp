#include "Common.hpp"

namespace bisky::core
{

const std::string float2(dx::XMFLOAT2 vector)
{
    return "[" + std::to_string(vector.x) + ", " + std::to_string(vector.y) + "]";
}

const std::string float3(dx::XMFLOAT3 vector)
{
    return "[" + std::to_string(vector.x) + ", " + std::to_string(vector.y) + ", " + std::to_string(vector.z) + "]";
}

const std::string float3(DirectX::XMVECTOR vector)
{
    return "[" + std::to_string(dx::XMVectorGetX(vector)) + ", " + std::to_string(dx::XMVectorGetY(vector)) + ", " +
           std::to_string(dx::XMVectorGetZ(vector)) + "]";
}

const std::string float4(dx::XMFLOAT4 vector)
{
    return "[" + std::to_string(vector.x) + ", " + std::to_string(vector.y) + ", " + std::to_string(vector.z) + ", " +
           std::to_string(vector.w) + "]";
}

const std::string float4(DirectX::XMVECTOR vector)
{
    return "[" + std::to_string(dx::XMVectorGetX(vector)) + ", " + std::to_string(dx::XMVectorGetY(vector)) + ", " +
           std::to_string(dx::XMVectorGetZ(vector)) + ", " + std::to_string(dx::XMVectorGetW(vector)) + "]";
}

} // namespace bisky::core