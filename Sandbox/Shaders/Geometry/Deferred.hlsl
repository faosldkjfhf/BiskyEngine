#include "Common.hlsli"

struct VsOutput
{
    float4 position : SV_Position;
    float3 positionW : POSITION0;
    float2 texCoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct PsOutput
{
    float4 positionAmbientOcclusion : SV_Target0;
    float4 normalRoughness : SV_Target1;
    float4 albedoMetallic : SV_Target2;
    float4 emissive : SV_Target3;
};

struct RenderResource
{
    int vertexBufferIndex;
    int diffuseMapIndex;
    int metallicRoughnessMapIndex;
    int ambientOcclusionMapIndex;
    int emissiveMapIndex;
};

ConstantBuffer<RenderResource> renderResource : register(b0);
ConstantBuffer<SceneBuffer> sceneBuffer : register(b1);
ConstantBuffer<ObjectBuffer> objectBuffer : register(b2);

VsOutput VsMain(uint vertexId : SV_VertexID)
{
    StructuredBuffer<Vertex> vertices = ResourceDescriptorHeap[renderResource.vertexBufferIndex];
    Vertex vertex = vertices[vertexId];
    
    VsOutput output;
    output.positionW = mul(float4(vertex.position, 1.0), objectBuffer.world).xyz;
    output.position = mul(float4(output.positionW, 1.0), sceneBuffer.viewProjection);
    output.texCoord = vertex.texCoord;
    output.normal = mul(vertex.normal, (float3x3) objectBuffer.transposeInverseWorld);
    return output;
}

PsOutput PsMain(VsOutput input)
{
    Texture2D<float4> diffuseMap = ResourceDescriptorHeap[renderResource.diffuseMapIndex];
    Texture2D<float4> metallicRoughnessMap = ResourceDescriptorHeap[renderResource.metallicRoughnessMapIndex];
    Texture2D<float4> ambientOcclusionMap = ResourceDescriptorHeap[renderResource.ambientOcclusionMapIndex];
    Texture2D<float4> emissiveMap = ResourceDescriptorHeap[renderResource.emissiveMapIndex];
    
    // -- emissive is optional
    float3 emissive = float3(0.0, 0.0, 0.0);
    if (renderResource.emissiveMapIndex > 0)
    {
        emissive = emissiveMap.Sample(linearWrapSampler, input.texCoord).rgb;
    }
 
    // ----- gamma correct diffuse texture -----
    float3 albedo = diffuseMap.Sample(linearWrapSampler, input.texCoord).xyz;
    albedo = pow(albedo, 2.2);
    
    // ----- gamma correct emissive texture -----
    emissive = pow(emissive, 2.2);
    
    PsOutput output;
    output.positionAmbientOcclusion.xyz = input.positionW;
    output.positionAmbientOcclusion.w = ambientOcclusionMap.Sample(linearWrapSampler, input.texCoord).r;
    output.normalRoughness.rgb = normalize(input.normal);
    output.normalRoughness.a = metallicRoughnessMap.Sample(linearWrapSampler, input.texCoord).g;
    output.albedoMetallic.rgb = albedo;
    output.albedoMetallic.a = metallicRoughnessMap.Sample(linearWrapSampler, input.texCoord).b;
    output.emissive.rgb = emissive;
    output.emissive.a = 1.0;
    return output;
}