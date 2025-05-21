#include "Common.hlsli"

struct VOutput
{
    float4 Position : SV_Position;
    float3 FragPosition : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

ConstantBuffer<ObjectConstants> gObject : register(b0);
ConstantBuffer<MaterialConstants> gMaterial : register(b1);
ConstantBuffer<PassConstants> gPass : register(b2);

VOutput main(VInput input)
{
    float4 position = float4(input.Position, 1.0);
    VOutput output = (VOutput) 0;
    output.Position = mul(gPass.Projection, mul(gPass.View, mul(gObject.World, position)));
    output.FragPosition = mul(gObject.World, position).xyz;
    output.Normal = mul((float3x3) gObject.NormalMatrix, input.Normal);
    output.TexCoord = input.TexCoord;
    return output;
}