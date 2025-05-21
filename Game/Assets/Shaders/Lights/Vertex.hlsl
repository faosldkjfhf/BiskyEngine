#include "Common.hlsli"

struct VOutput
{
    float4 Position : SV_Position;
    float3 Color : COLOR;
};

ConstantBuffer<ObjectConstants> gObject : register(b0);
ConstantBuffer<MaterialConstants> gMaterial : register(b1);
ConstantBuffer<PassConstants> gPass : register(b2);

VOutput main(VInput input)
{
    VOutput output = (VOutput) 0;
    output.Position = mul(gPass.Projection, mul(gPass.View, mul(gObject.World, float4(input.Position, 1.0))));
    output.Color = float3(1.0, 1.0, 1.0);
    return output;
}