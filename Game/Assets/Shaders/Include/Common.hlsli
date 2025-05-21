#define NUM_LIGHTS 1

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