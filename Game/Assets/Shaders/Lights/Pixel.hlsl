struct VOutput
{
    float4 Position : SV_Position;
    float3 Color : COLOR;
};

float4 main(VOutput input) : SV_Target
{
    return float4(input.Color, 1.0);
}