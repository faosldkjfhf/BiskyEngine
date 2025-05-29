#include "Common.hlsli"

/*
* Shlick's Appromixation for the Fresnel Equation
*
* @param F0 The amount of light reflected back at normal incidence (0)
* @param u Cosine of the angle between V and H
*/
float3 F_FresnelShlick(float3 F0, float u)
{
    float F = pow(1.0 - u, 5.0);
    return F + F0 * (1.0 - F);
}

/*
* Microfacet Distribution Function (GGX)
*
* @param NoH The dot product between normal and halfway vector
* @param roughness The material roughness
*/
float D_GGX(float NoH, float roughness)
{
    float a = NoH * roughness;
    float k = roughness / (1.0 - NoH * NoH + a * a);
    return k * k * (1.0 / M_PI);
}

/*
* Smith Geometry Function (GGX)
*
* @param NoV Dot product between normal and outgoing light
* @param NoL Dot product between normal and incident light
* @param roughness The material roughness
*/
float SmithG_GGX(float NoV, float NoL, float roughness)
{
    float a = roughness;
    float GGXV = NoL * (NoV * (1.0 - a) + a);
    float GGXL = NoV * (NoL * (1.0 - a) + a);
    return 0.5 / (GGXV + GGXL + 0.00001);
}

float Fd_Lambert()
{
    return 1.0 / M_PI;
}

float3 BRDF_CookTorrance()
{
    return float3(0.0, 0.0, 0.0);
}