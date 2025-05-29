#include "Common.hlsli"

struct SkyboxOutput
{
    float4 PositionH : SV_Position;
    float3 PositionL : POSITION;
};

TextureCube gCubeMap : register(t0);

float4 main(SkyboxOutput input) : SV_TARGET
{
    return gCubeMap.Sample(gLinearSampler, input.PositionL);
}