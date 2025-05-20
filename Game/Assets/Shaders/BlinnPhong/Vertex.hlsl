struct VInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
};

struct VOutput
{
    float4 Position : SV_Position;
    float3 FragPosition : POSITION;
    float3 Normal : NORMAL;
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

ConstantBuffer<ObjectConstants> gObject : register(b0);
ConstantBuffer<PassConstants> gPass : register(b1);

VOutput main(VInput input)
{
    float4 position = float4(input.Position, 1.0);
    VOutput output = (VOutput) 0;
    output.Position = mul(gPass.Projection, mul(gPass.View, mul(gObject.World, position)));
    output.FragPosition = mul(gObject.World, position);
    output.Normal = mul((float3x3) gObject.NormalMatrix, input.Normal);
    return output;
}