#define NUM_LIGHTS 1

struct VInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

struct ObjectConstants
{
    float4x4 World;
    float4x4 InverseWorld;
    float4x4 NormalMatrix;
};

struct Light
{
    float4 Position;
    float4 Strength;
};

struct PassConstants
{
    float4x4 View;
    float4x4 Projection;
    float4 ViewPosition;
    Light Lights[1];
};

struct MaterialConstants
{
    float3 Diffuse;
    bool UseMaterial;
};

SamplerState gLinearSampler : register(s0);