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
Texture2D gAmbientOcclusionMap : register(t2);
Texture2D gMetalRoughnessMap : register(t3);
ConstantBuffer<ObjectConstants> gObject : register(b0);
ConstantBuffer<MaterialConstants> gMaterial : register(b1);
ConstantBuffer<PassConstants> gPass : register(b2);

/*
* Basic model
* I_r = I_ia * R_a + sum(I_il(dot(N, L_l) * dw_il * (sR_s + dR_d))
*
* Disney Model:
* Cook-Torrance specular microfacet model
* Lambertian diffuse model
*/
float4 main(VOutput input) : SV_TARGET
{
    float3 N = normalize(input.Normal);
    float3 V = normalize(gPass.ViewPosition.xyz - input.FragPosition);
    
    float3 Lo = float3(0.0, 0.0, 0.0);
    
    float roughness = 0.0;
    float metallic = 0.0;
    float ambientOcclusion = 0.0;
    float3 albedo = 0.0;
    if (gMaterial.UseMaterial)
    {
        roughness = gMaterial.Roughness;
        metallic = gMaterial.Metallic;
        albedo = gMaterial.Diffuse.xyz;
        ambientOcclusion = gMaterial.AmbientOcclusion;
    }
    else
    {
        roughness = gMetalRoughnessMap.Sample(gLinearSampler, input.TexCoord).g;
        metallic = gMetalRoughnessMap.Sample(gLinearSampler, input.TexCoord).b;
        albedo = gDiffuseMap.Sample(gLinearSampler, input.TexCoord).xyz;
        ambientOcclusion = gAmbientOcclusionMap.Sample(gLinearSampler, input.TexCoord).r;
        
        N = normalize(gNormalMap.Sample(gLinearSampler, input.TexCoord).xyz);
        N = N * 2.0 - 1.0;
        N = (N.x * input.tTangent) + (N.y * input.tBitangent) + (N.z * input.tNormal);
    }
    
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo, metallic);
    
    // For Cube_0, since it doesn't have a normal or ao map
    N = normalize(input.Normal);
    ambientOcclusion = gMaterial.AmbientOcclusion;
    
    roughness = sqr(roughness);
  
    for (int i = 0; i < NUM_POINT_LIGHTS; i++)
    {
        PointLight light = gPass.PointLights[i];
        
        float3 L = normalize(light.Position.xyz - input.FragPosition);
        float3 H = normalize(V + L);
        
        float NoV = abs(dot(N, V)) + 1e-5;
        float NoL = saturate(dot(N, L));
        float NoH = saturate(dot(N, H));
        float LoH = saturate(dot(L, H));
        
        float distance = length(light.Position.xyz - input.FragPosition);
        float attenuation = 1.0 / sqr(distance);
        float3 radiance = light.Strength.xyz * attenuation;
        
        float3 F = F_FresnelShlick(F0, LoH);
        float D = D_GGX(NoH, roughness);
        float G = SmithG_GGX(NoV, NoL, roughness);
        
        // specular BRDF
        float3 Fr = (F * D * G) / (4 * NoL * NoV + 0.00001);
        float3 Ft = float3(0.0, 0.0, 0.0);
        float3 Fs = Fr + Ft;
        
        // diffuse BRDF
        float3 Fd = albedo * Fd_Lambert();
        
        // R = sR_s + dR_d
        // R_s = specular reflectance
        // R_d = diffuse reflectance
        // s + d = 1
        float3 kS = F;
        float3 kD = 1.0 - F;
        kD *= 1.0 - metallic;

        // FIXME: There is a white ring around the edge on rough materials
        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * Fd + kS * Fs) * radiance * NdotL;
    }
    
    for (int i = 0; i < NUM_DIRECTIONAL_LIGHTS; i++)
    {
        DirectionalLight light = gPass.DirectionalLights[i];
        float3 L = normalize(light.Direction.xyz - input.FragPosition);
        float3 H = normalize(V + L);
        
        float NoV = abs(dot(N, V)) + 1e-5;
        float NoL = saturate(dot(N, L));
        float NoH = saturate(dot(N, H));
        float LoH = saturate(dot(L, H));
        
        float3 F = F_FresnelShlick(F0, LoH);
        float D = D_GGX(NoH, roughness);
        float G = SmithG_GGX(NoV, NoL, roughness);
        
        // specular BRDF
        float3 Fr = (F * D * G) / (4 * NoL * NoV + 0.00001);
        float3 Ft = float3(0.0, 0.0, 0.0);
        float3 Fs = Fr + Ft;
        
        // diffuse BRDF
        float3 Fd = albedo * Fd_Lambert();
        
        // R = sR_s + dR_d
        // R_s = specular reflectance
        // R_d = diffuse reflectance
        // s + d = 1
        float3 kS = F;
        float3 kD = 1.0 - F;
        kD *= 1.0 - metallic;

        // FIXME: There is a white ring around the edge on rough materials
        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * Fd + kS * Fs) * NdotL;
    }
    
    float3 ambient = 0.03 * albedo * ambientOcclusion;
    float3 color = ambient + Lo;
    
    // gamma correction and tone mapping
    color = color / (color + 1.0);
    color = pow(color, 1.0 / 2.2);
    
    return float4(color, 1.0);
}