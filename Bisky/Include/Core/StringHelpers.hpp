#pragma once

#include <DirectXMath.h>
#include <string>

/*
 * Helpers for debugging DirectX structs
 */
namespace bisky::core
{

const std::string float2(DirectX::XMFLOAT2 vector);
const std::string float3(DirectX::XMFLOAT3 vector);
const std::string float3(DirectX::XMVECTOR vector);
const std::string float4(DirectX::XMFLOAT4 vector);
const std::string float4(DirectX::XMVECTOR vector);

} // namespace bisky::core