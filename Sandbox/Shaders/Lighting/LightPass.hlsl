#include "Lighting/PBR.hlsli"
#include "Lighting/BlinnPhong.hlsli"

struct VsOutput
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD0;
};

struct PsOutput
{
    float4 color : SV_Target0;
    float4 bloomColor : SV_Target1;
};

struct RawVertex
{
    float3 position : POSITION0;
    float2 texCoord : TEXCOORD0;
};

struct RenderResource
{
    int vertexBufferIndex;
    int positionAmbientOcclusionTextureIndex;
    int normalRoughnessTextureIndex;
    int albedoMetallicTextureIndex;
    int emissiveTextureIndex;
};

ConstantBuffer<RenderResource> renderResource : register(b0);
ConstantBuffer<SceneBuffer> sceneBuffer : register(b1);
ConstantBuffer<LightBuffer> lightBuffer : register(b2);

VsOutput VsMain(uint vertexId : SV_VertexID)
{
    StructuredBuffer<RawVertex> vertices = ResourceDescriptorHeap[renderResource.vertexBufferIndex];
    RawVertex vertex = vertices[vertexId];
    
    VsOutput output;
    output.position = float4(vertex.position, 1.0);
    output.texCoord = vertex.texCoord;
    return output;
}

PsOutput PsMain(VsOutput input)
{
    Texture2D<float4> positionAmbientOcclusionMap = ResourceDescriptorHeap[NonUniformResourceIndex(renderResource.positionAmbientOcclusionTextureIndex)];
    Texture2D<float4> normalRoughnessMap = ResourceDescriptorHeap[NonUniformResourceIndex(renderResource.normalRoughnessTextureIndex)];
    Texture2D<float4> albedoMetallicMap = ResourceDescriptorHeap[NonUniformResourceIndex(renderResource.albedoMetallicTextureIndex)];
    Texture2D<float4> emissiveMap = ResourceDescriptorHeap[NonUniformResourceIndex(renderResource.emissiveTextureIndex)];
    
    float4 normalRoughness = normalRoughnessMap.Sample(linearWrapSampler, input.texCoord);
    float4 albedoMetallic = albedoMetallicMap.Sample(linearWrapSampler, input.texCoord);
    float4 positionAmbientOcclusion = positionAmbientOcclusionMap.Sample(linearWrapSampler, input.texCoord);
    
    float3 position = positionAmbientOcclusion.xyz;
    float ambientOcclusion = positionAmbientOcclusion.w;
    float3 normal = normalRoughness.xyz;
    float roughness = normalRoughness.w;
    float3 albedo = albedoMetallic.xyz;
    float metalness = albedoMetallic.w;
    
       // ----- Normal vector and View direction -----
    float3 N = normalize(normal);
    float3 V = normalize(sceneBuffer.viewPosition.xyz - position);
    
    // ----- Calculate f0 -----
    float3 f0 = float3(0.04, 0.04, 0.04);
    f0 = lerp(f0, albedo, metalness);
    
    // ----- Accumulate lighting -----
    float3 Lo = float3(0.0, 0.0, 0.0);
    for (uint i = 0; i < lightBuffer.numLights; i++)
    {
        Light light = lightBuffer.lights[i];
       
        // ----- Light direction and Halfway direction -----
        float3 L = normalize(light.position.xyz - position);
        float3 H = normalize(L + V);
        
        // ----- Attenuation -----
        float distance = length(light.position.xyz - position);
        float attenuation = 1.0 / (distance * distance);
        float3 radiance = light.strength.xyz * attenuation;
        
        // ----- Calculate DGF -----
        float D = D_GGX(N, H, roughness);
        float G = G_Smith(N, V, L, roughness);
        float3 F = F_FresnelSchlick(max(dot(L, H), 0.0), f0);
        
        // ----- Calculate the diffuse contribution -----
        float3 kD = 1.0 - F;
        kD *= 1.0 - metalness;
        
        // ----- Specular component -----
        float3 num = D * G * F;
        float3 denom = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 1e-5;
        float3 Fs = num / denom;
        
        // ----- Diffuse component (Lambertian) -----
        float3 Fd = albedo / PI;
        
        // ----- Accumulate outgoing light -----
        float NoL = max(dot(N, L), 0.0);
        Lo += (kD * Fd + (1.0 - kD) * Fs) * NoL * radiance;
        
    }
    
    // ----- Calculate ambient -----
    float3 ambient = ambientOcclusion * albedo * 0.03;
    Lo += ambient + emissiveMap.Sample(linearWrapSampler, input.texCoord).rgb;
   
    PsOutput output;
    output.color = float4(Lo, 1.0);
    
    // -- if brightness is greater than a certain threshold
    // -- output the color to the bloom texture
    float brightness = dot(Lo, float3(0.2126, 0.7152, 0.07722));
    if (brightness > 1.0)
    {
        output.bloomColor = float4(Lo, 1.0);
    }
    else
    {
        output.bloomColor = float4(0.0, 0.0, 0.0, 1.0);
    }
    
    return output;
}