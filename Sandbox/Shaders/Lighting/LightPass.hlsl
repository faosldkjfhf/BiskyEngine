#include "Common.hlsli"

struct VsOutput
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD0;
};

struct PsOutput
{
    float4 color : SV_Target0;
};

struct RawVertex
{
    float3 position;
    float2 texCoord;
};

struct RenderResource
{
    int vertexBufferIndex;
    int positionTextureIndex;
    int normalTextureIndex;
    int albedoTextureIndex;
};

ConstantBuffer<RenderResource> renderResource : register(b0);
ConstantBuffer<SceneBuffer> sceneBuffer : register(b1);
ConstantBuffer<LightBuffer> lightBuffer : register(b2);

VsOutput VsMain(uint vertexId : SV_VertexID)
{
    StructuredBuffer<RawVertex> vertices = ResourceDescriptorHeap[renderResource.vertexBufferIndex];
    
    VsOutput output;
    output.position = float4(vertices[vertexId].position, 1.0);
    output.texCoord = vertices[vertexId].texCoord;
    return output;
}

PsOutput PsMain(VsOutput input)
{
    Texture2D<float4> positionMap = ResourceDescriptorHeap[renderResource.positionTextureIndex];
    Texture2D<float4> normalMap = ResourceDescriptorHeap[renderResource.normalTextureIndex];
    Texture2D<float4> albedoMap = ResourceDescriptorHeap[renderResource.albedoTextureIndex];
    
    float3 position = positionMap.Sample(linearWrapSampler, input.texCoord).xyz;
    float3 normal = normalMap.Sample(linearWrapSampler, input.texCoord).xyz;
    float3 albedo = albedoMap.Sample(linearWrapSampler, input.texCoord).xyz;
    
    float3 Lo = float3(0.0, 0.0, 0.0);
    for (int i = 0; i < lightBuffer.numLights; i++)
    {
        Light light = lightBuffer.lights[i];
        
        float ambientStrength = 0.3;
        float3 ambient = ambientStrength * light.strength.xyz;
        
        float3 N = normalize(normal);
        float3 L = normalize(light.position.xyz - position);
        float3 NoL = max(dot(N, L), 0.0);
        float3 diffuse = NoL * light.strength.xyz;
        
        float3 V = normalize(sceneBuffer.viewPosition.xyz - position);
        float3 H = normalize(L + V);
        float spec = pow(max(dot(N, H), 0.0), 16.0);
        float3 specular = spec * light.strength.xyz;
        
        Lo += ambient + diffuse + specular;
    }
    
    PsOutput output;
    output.color = float4(Lo * albedo, 1.0);
    return output;
}