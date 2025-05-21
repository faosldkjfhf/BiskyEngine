#include "Common.hlsli"

struct VOutput
{
    float4 Position : SV_Position;
    float3 FragPosition : POSITION;
    float3 Normal : NORMAL;
};

ConstantBuffer<ObjectConstants> gObject : register(b0);
ConstantBuffer<MaterialConstants> gMaterial : register(b1);
ConstantBuffer<PassConstants> gPass : register(b2);

// FIXME: Pass in lights through pass constants
float4 main(VOutput input) : SV_Target {
    float4 finalColor = float4(0.0, 0.0, 0.0, 1.0);
    for (int i = 0; i < NUM_LIGHTS; i++)
    {
        Light light = gPass.Lights[i];
        float ambientStrength = 0.3;
        float3 ambient = ambientStrength * light.Strength.xyz;
        
        float3 normal = normalize(input.Normal);
        float3 lightDir = normalize(light.Position.xyz - input.FragPosition);
        float diff = max(dot(normal, lightDir), 0.0);
        float3 diffuse = diff * light.Strength.xyz;
        
        float3 viewDir = normalize(gPass.ViewPosition.xyz - input.FragPosition);
        float3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(halfwayDir, normal), 0.0), 8);
        float3 specular = spec * light.Strength.xyz;
        
        float3 color = (ambient + diffuse + specular) * gMaterial.Diffuse;
        finalColor += float4(color, 0.0);
    }
    
    return finalColor;
}