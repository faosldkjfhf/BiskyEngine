#include "Common.hlsli"

struct VOutput
{
    float4 Position : SV_Position;
    float3 FragPosition : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
    float3 Debug : COLOR;
    float3x3 TBNMatrix : TANGENT1;
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
    
    float3 t = normalize(mul(gObject.World, float4(input.Tangent, 0.0)).xyz);
    float3 n = normalize(mul(gObject.World, float4(input.Normal, 0.0)).xyz);
    float3 b = normalize(mul(gObject.World, float4(input.Bitangent, 0.0)).xyz);
    
    // FIXME: Figure out how to correctly calculate bitangents - right now they are passed in
    
    output.Debug = b;
    output.TBNMatrix = transpose(float3x3(t, b, n));
    
    return output;  
}