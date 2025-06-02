#include "Common.hlsli"

struct VOutput
{
    float4 position : SV_Position;
    float3 positionL : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
};

struct Vertex
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float4 tangent : TANGENT;
};

ConstantBuffer<RenderResource> renderResource : register(b0);
ConstantBuffer<ObjectBuffer> objectBuffer : register(b1);
SamplerState gLinearWrap : register(s0);

VOutput vertexMain(uint vertexId : SV_VertexID)
{
    StructuredBuffer<Vertex> vertexBuffer = ResourceDescriptorHeap[renderResource.vertexBufferIndex];
    ConstantBuffer<SceneBuffer> sceneBuffer = ResourceDescriptorHeap[renderResource.sceneBufferIndex];
    Vertex vertex = vertexBuffer[vertexId];
    
    VOutput output = (VOutput) 0;
    
    output.positionL = mul(float4(vertex.position, 1.0), objectBuffer.world).xyz;
    output.position = mul(float4(output.positionL, 1.0), sceneBuffer.viewProjection);
    output.normal = vertex.normal;
    output.texCoord = vertex.texCoord;
    
    return output;
}

float4 pixelMain(VOutput input) : SV_Target
{
    Texture2D diffuse = ResourceDescriptorHeap[renderResource.diffuseTextureIndex];
    Texture2D metalRoughness = ResourceDescriptorHeap[renderResource.metallicRoughnessTextureIndex];
    
    return float4(diffuse.Sample(gLinearWrap, input.texCoord).xyz, 1.0);
}