struct VInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
};

struct VOutput
{
    float4 Position : SV_Position;
    float3 Color : COLOR;
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
    VOutput output = (VOutput) 0;
    output.Position = mul(gPass.Projection, mul(gPass.View, mul(gObject.World, float4(input.Position, 1.0))));
    output.Color = float3(1.0, 1.0, 1.0);
    return output;
}