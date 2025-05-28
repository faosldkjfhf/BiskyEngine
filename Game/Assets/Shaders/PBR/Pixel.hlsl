#include "Lighting.hlsli"

struct VOutput
{
    float4 Position : SV_Position;
    float3 FragPosition : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
    float3 Debug : COLOR;
    float3 tTangent : TANGENT0;
    float3 tNormal : TANGENT1;
    float3 tBitangent : TANGENT2;
};

Texture2D gDiffuseMap : register(t0);
Texture2D gNormalMap : register(t1);
ConstantBuffer<ObjectConstants> gObject : register(b0);
ConstantBuffer<MaterialConstants> gMaterial : register(b1);
ConstantBuffer<PassConstants> gPass : register(b2);

/*
* Basic model
* I_r = I_ia * R_a + sum(I_il(dot(N, L_l) * dw_il * (sR_s + dR_d))
*/
float4 main(VOutput input) : SV_TARGET
{
    float3 N = normalize(input.Normal);
    float3 V = normalize(gPass.ViewPosition.xyz - input.FragPosition);
    
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, gMaterial.Diffuse, gMaterial.Metallic);
    
    float3 Lo = float3(0.0, 0.0, 0.0);
    for (int i = 0; i < NUM_LIGHTS; i++)
    {
        PointLight light = gPass.PointLights[i];
        
        float3 L = normalize(light.Position.xyz - input.FragPosition);
        float3 H = normalize(V + L);
        float cosTheta = dot(V, H); // should be same as dot(L, H) since H is a bisector
        float distance = length(light.Position.xyz - input.FragPosition);
        float attenuation = 1.0 / sqr(distance);
        float3 radiance = light.Strength.xyz * attenuation;
        
        float3 F = F_FresnelShlick(F0, cosTheta);
        float D = D_GGX(max(dot(N, H), 0.0), gMaterial.Roughness);
        float G = SmithG_GGX(N, V, L, gMaterial.Roughness);
        
        float3 numerator = F * D * G;
        float3 denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.000001;
        float3 specular = numerator / denominator;
        
        // R = sR_s + dR_d
        // R_s = specular reflectance
        // R_d = diffuse reflectance
        // s + d = 1
        float3 kS = F;
        float3 kD = float3(1.0, 1.0, 1.0) - F;
        kD *= 1.0 - gMaterial.Metallic;

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * gMaterial.Diffuse / M_PI + specular) * radiance * NdotL;
    }
    
    float3 ambient = float3(0.03, 0.03, 0.03) * gMaterial.Diffuse * gMaterial.AmbientOcclusion;
    float3 color = ambient + Lo;
    
    color = color / (color + float3(1.0, 1.0, 1.0));
    color = pow(color, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
    
    return float4(color, 1.0);
}