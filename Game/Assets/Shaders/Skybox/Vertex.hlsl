#include "Common.hlsli"

struct SkyboxOutput
{
    float4 PositionH : SV_Position;
    float3 PositionL : POSITION;
};

ConstantBuffer<ObjectConstants> gObject : register(b0);
ConstantBuffer<PassConstants> gPass : register(b1);

SkyboxOutput main(VInput input)
{
    SkyboxOutput output = (SkyboxOutput) 0;
    
    output.PositionL = input.Position;
    
    float4x4 view =
    {
        { gPass.View[0].xyz, 0.0 },
        { gPass.View[1].xyz, 0.0 },
        { gPass.View[2].xyz, 0.0 },
        { 0.0, 0.0, 0.0, 1.0 },
    };
    // float4 positionW = mul(gObject.World, float4(input.Position, 1.0));
    // positionW.xyz += gPass.ViewPosition.xyz;
    // output.PositionH = mul(gPass.Projection, mul(gPass.View, positionW)).xyzw;
    
    output.PositionH = mul(gPass.Projection, mul(view, float4(input.Position, 1.0))).xyww;
    
    return output;
}