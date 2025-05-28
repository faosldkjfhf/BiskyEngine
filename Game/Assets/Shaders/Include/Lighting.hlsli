#include "Common.hlsli"

/*
* Shlick's Appromixation for the Fresnel Equation
*
* @param F0 amount of light reflected back at normal incidence (0)
* @param u cosine of the angle between V and H
*/
float3 F_FresnelShlick(float3 F0, float u)
{
    float F = pow(clamp(1.0 - u, 0.0, 1.0), 5.0);
    return F + F0 * (1.0 - F);
}

/*
* Microfacet Distribution Function (GGX)
*/
float D_GGX(float NoH, float roughness)
{
    float a = NoH * roughness;
    float k = roughness / (1.0 - NoH * NoH + a * a);
    return k * k * (1.0 / M_PI);
}

/*
*
*/
float ShlickGGX(float NdotV, float k)
{
    float denom = NdotV * (1.0 - k) + k;
    return NdotV / denom;
}

/*
*
*/
float SmithG_GGX(float3 N, float3 V, float3 L, float K)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return ShlickGGX(NdotV, K) * ShlickGGX(NdotL, K);
}