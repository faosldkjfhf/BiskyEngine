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
    float4 position : SV_Target0;
    float4 normal : SV_Target1;
    float4 albedo : SV_Target2;
};

struct RenderResource
{
    int vertexBufferIndex;
    int diffuseMapIndex;
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
    
    float3 albedo = diffuseMap.Sample(linearWrapSampler, input.texCoord).xyz;
    albedo = pow(albedo, 2.2);
    
    PsOutput output;
    output.position = float4(input.positionW, 1.0);
    output.normal = float4(normalize(input.normal), 1.0);
    output.albedo = float4(albedo, 1.0);
    return output;
}