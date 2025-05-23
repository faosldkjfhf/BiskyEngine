#include "Common.hlsli"

struct VOutput
{
    float4 Position : SV_Position;
    float3 FragPosition : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
    float3 Tangent : TANGENT0;
    float3x3 TBNMatrix : TANGENT1;
};

Texture2D gDiffuseMap : register(t0);
Texture2D gNormalMap : register(t1);
ConstantBuffer<ObjectConstants> gObject : register(b0);
ConstantBuffer<MaterialConstants> gMaterial : register(b1);
ConstantBuffer<PassConstants> gPass : register(b2);

// FIXME: Pass in lights through pass constants
float4 main(VOutput input) : SV_Target {
    float4 finalColor = float4(0.0, 0.0, 0.0, 1.0);
    
    float3 color = gMaterial.UseMaterial ? gMaterial.Diffuse : gDiffuseMap.Sample(gLinearSampler, input.TexCoord).xyz;
    
    float3 normal = gMaterial.UseMaterial ? normalize(input.Normal) : normalize(gNormalMap.Sample(gLinearSampler, input.TexCoord).xyz);
    normal.y = 1.0 - normal.y; // flip the normal y - glTF normal maps follow right-handed convention instead of left-handed
    normal = normal * 2.0 - 1.0;
    normal = normalize(mul(input.TBNMatrix, normal));
    
    float3 viewDir = normalize(gPass.ViewPosition.xyz - input.FragPosition);
    
    // attenuation values
    float gConstant = 1.0;
    float gLinear = 0.14;
    float gQuadratic = 0.07;
    
    for (int i = 0; i < NUM_LIGHTS; i++)
    {
        PointLight light = gPass.PointLights[i];
        float ambientStrength = 0.3;
        float3 ambient = ambientStrength * light.Strength.xyz;
        
        float3 lightDir = normalize(light.Position.xyz - input.FragPosition);
        float diff = max(dot(normal, lightDir), 0.0);
        float3 diffuse = diff * light.Strength.xyz;
        
        float3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(halfwayDir, normal), 0.0), 32);
        float3 specular = spec * light.Strength.xyz;
        
        float distance = length(light.Position.xyz - input.FragPosition);
        float attenuation = 1.0 / (gConstant + gLinear * distance + gQuadratic * (distance * distance));
        
        ambient *= attenuation;
        diffuse *= attenuation;
        specular *= attenuation;
        
        float3 result = (ambient + diffuse + specular) * color;
        finalColor += float4(result, 0.0);
    }
    
    // gamma correction
    // FIXME: do it in a post processing step
    float gamma = 2.2;
    finalColor.rgb = pow(finalColor.rgb, float3(1.0 / gamma, 1.0 / gamma, 1.0 / gamma));
    return finalColor;
}