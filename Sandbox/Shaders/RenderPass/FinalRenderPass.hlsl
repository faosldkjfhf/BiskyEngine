#include "Common.hlsli"

struct Vertex
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD;
};

struct VSOutput
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

VSOutput VsMain(uint vertexId : SV_VertexID)
{
    StructuredBuffer<Vertex> vertices = ResourceDescriptorHeap[renderResource.vertexBufferIndex];
    
    VSOutput output;
    
    output.position = float4(vertices[vertexId].position, 1.0);
    output.texCoord = vertices[vertexId].texCoord;
    
    return output;
}

float4 PsMain(VSOutput input) : SV_Target
{
    Texture2D<float4> rtvTexture = ResourceDescriptorHeap[renderResource.textureIndex];
    
    return rtvTexture.SampleLevel(linearWrapSampler, input.texCoord, 0.0);
}