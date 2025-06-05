#include "Common.hlsli"

struct Vertex
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD;
};

struct VsOutput
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD;
};

struct RenderResource
{
    int vertexBufferIndex;
    int textureIndex;
};


ConstantBuffer<RenderResource> renderResource : register(b0);

VsOutput VsMain(uint vertexId : SV_VertexID)
{
    StructuredBuffer<Vertex> vertices = ResourceDescriptorHeap[renderResource.vertexBufferIndex];
    
    VsOutput output;
    output.position = float4(vertices[vertexId].position, 1.0);
    output.texCoord = vertices[vertexId].texCoord;
    return output;
}

float4 PsMain(VsOutput input) : SV_Target
{
    Texture2D<float4> rtvTexture = ResourceDescriptorHeap[renderResource.textureIndex];
    
    float3 hdr = rtvTexture.SampleLevel(linearWrapSampler, input.texCoord, 0.0).xyz;
    return float4(hdr, 1.0);
}