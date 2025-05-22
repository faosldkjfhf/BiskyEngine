#define NUM_LIGHTS 1

struct VInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
    float3 Tangent : TANGENT;
};

struct ObjectConstants
{
    float4x4 World;
    float4x4 InverseWorld;
    float4x4 NormalMatrix;
};

struct PointLight
{
    float4 Position;
    float4 Strength;
};

struct PassConstants
{
    float4x4 View;
    float4x4 Projection;
    float4 ViewPosition;
    PointLight PointLights[1];
};

struct MaterialConstants
{
    float3 Diffuse;
    bool UseMaterial;
};

SamplerState gLinearSampler : register(s0);