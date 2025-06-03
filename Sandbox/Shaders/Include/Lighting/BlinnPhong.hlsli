#pragma once

#include "Common.hlsli"

float3 BlinnPhong(Light light, float3 normal, float3 positionW, float3 viewPositionW)
{
    float ambientStrength = 0.3;
    float3 ambient = ambientStrength * light.strength.xyz;
        
    float3 N = normalize(normal);
    float3 L = normalize(light.position.xyz - positionW);
    float3 NoL = max(dot(N, L), 0.0);
    float3 diffuse = NoL * light.strength.xyz;
        
    float3 V = normalize(viewPositionW - positionW);
    float3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), 16.0);
    float3 specular = spec * light.strength.xyz;
    
    return ambient + diffuse + specular;
}