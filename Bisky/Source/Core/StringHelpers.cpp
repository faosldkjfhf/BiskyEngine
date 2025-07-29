#include "Common.hpp"

namespace bisky::core
{

const std::string float2(XMFLOAT2 vector)
{
    return "[" + std::to_string(vector.x) + ", " + std::to_string(vector.y) + "]";
}

const std::string float3(XMFLOAT3 vector)
{
    return "[" + std::to_string(vector.x) + ", " + std::to_string(vector.y) + ", " + std::to_string(vector.z) + "]";
}

const std::string float3(DirectX::XMVECTOR vector)
{
    return "[" + std::to_string(XMVectorGetX(vector)) + ", " + std::to_string(XMVectorGetY(vector)) + ", " +
           std::to_string(XMVectorGetZ(vector)) + "]";
}

const std::string float4(XMFLOAT4 vector)
{
    return "[" + std::to_string(vector.x) + ", " + std::to_string(vector.y) + ", " + std::to_string(vector.z) + ", " +
           std::to_string(vector.w) + "]";
}

const std::string float4(DirectX::XMVECTOR vector)
{
    return "[" + std::to_string(XMVectorGetX(vector)) + ", " + std::to_string(XMVectorGetY(vector)) + ", " +
           std::to_string(XMVectorGetZ(vector)) + ", " + std::to_string(XMVectorGetW(vector)) + "]";
}

} // namespace bisky::core