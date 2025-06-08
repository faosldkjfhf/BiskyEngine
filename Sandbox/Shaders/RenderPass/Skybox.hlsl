#include "Common.hlsli"

struct Vertex
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float4 tangent : TANGENT;
};

struct VsOutput
{
    float4 position : SV_Position;
    float3 positionW : POSITION;
};

struct RenderResource
{
    int vertexBufferIndex;
    int textureIndex;
};

ConstantBuffer<SceneBuffer> sceneBuffer: register(b0);
ConstantBuffer<RenderResource> renderResource : register(b1);

VsOutput VsMain(uint vertexId : SV_VertexID)
{
    StructuredBuffer<Vertex> vertices = ResourceDescriptorHeap[renderResource.vertexBufferIndex];
    
    VsOutput output;
    output.positionW = vertices[vertexId].position;
    
    float4x4 view =
    {
        { sceneBuffer.view[0].xyz, 0.0 },
        { sceneBuffer.view[1].xyz, 0.0 },
        { sceneBuffer.view[2].xyz, 0.0 },
        { 0.0, 0.0, 0.0, 1.0 },
    };
    
    output.position = mul(float4(vertices[vertexId].position, 1.0), mul(view, sceneBuffer.projection)).xyww;
    return output;
}

float4 PsMain(VsOutput input) : SV_Target
{
    TextureCube skybox = ResourceDescriptorHeap[renderResource.textureIndex];
    
    return skybox.Sample(linearWrapSampler, input.positionW);
}