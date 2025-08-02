#pragma once

#include "Common.hlsli"

float3 F_FresnelSchlick(float u, float3 f0)
{
    return f0 + (1.0 - f0) * pow(saturate(1.0 - u), 5.0);
}

float D_GGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NoH = max(dot(N, H), 0.0);
    float NoH2 = NoH * NoH;
    
    float num = a2;
    float denom = (NoH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return num / denom;
}

float G_SchlickGGX(float NoV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    
    float num = NoV;
    float denom = NoV * (1.0 - k) + k;
    
    return num / denom;
}

float G_Smith(float3 N, float3 V, float3 L, float roughness)
{
    float NoV = max(dot(N, V), 0.0);
    float NoL = max(dot(N, L), 0.0);
    float ggx1 = G_SchlickGGX(NoV, roughness);
    float ggx2 = G_SchlickGGX(NoL, roughness);
    return ggx1 * ggx2;
}