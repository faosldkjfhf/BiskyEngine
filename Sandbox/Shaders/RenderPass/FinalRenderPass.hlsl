#include "Common.hlsli"

struct SimpleVertex
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
    StructuredBuffer<SimpleVertex> vertices = ResourceDescriptorHeap[renderResource.vertexBufferIndex];
    
    VsOutput output;
    output.position = float4(vertices[vertexId].position, 1.0);
    output.texCoord = vertices[vertexId].texCoord;
    return output;
}

float4 PsMain(VsOutput input) : SV_Target0
{
    Texture2D<float4> rtvTexture = ResourceDescriptorHeap[renderResource.textureIndex];
    
    float3 hdr = rtvTexture.SampleLevel(linearWrapSampler, input.texCoord, 0.0).xyz;
    
    // ----- Reinhard tone-mapping -----
    hdr = hdr / (hdr + 1.0);
    
    // ----- Gamma correction -----
    hdr = pow(hdr, 1.0 / 2.2);
    
    return float4(hdr, 1.0);
}