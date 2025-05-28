#define NUM_LIGHTS 1
#define M_PI 3.14159265358979323846

float sqr(float x)
{
    return x * x;
}

struct VInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
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
    float Metallic;
    float Roughness;
    float AmbientOcclusion;
    bool UseMaterial;
    float Buffer1;
};

SamplerState gLinearSampler : register(s0);