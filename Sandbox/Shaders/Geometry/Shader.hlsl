#include "Common.hlsli"
#include "Lighting/BlinnPhong.hlsli"

struct VOutput
{
    float4 position : SV_Position;
    float3 positionW : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float3 tangent : TANGENT;
};

struct Vertex
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float4 tangent : TANGENT;
};

struct RenderResource
{
    int vertexBufferIndex;
    int sceneBufferIndex;
    int diffuseTextureIndex;
    int metallicRoughnessTextureIndex;
};

ConstantBuffer<RenderResource> renderResource : register(b0);
ConstantBuffer<ObjectBuffer> objectBuffer : register(b1);
ConstantBuffer<LightBuffer> lightBuffer : register(b2);
SamplerState gLinearWrap : register(s0);

VOutput VsMain(uint vertexId : SV_VertexID)
{
    StructuredBuffer<Vertex> vertexBuffer = ResourceDescriptorHeap[renderResource.vertexBufferIndex];
    ConstantBuffer<SceneBuffer> sceneBuffer = ResourceDescriptorHeap[renderResource.sceneBufferIndex];
    Vertex vertex = vertexBuffer[vertexId];
    
    VOutput output = (VOutput) 0;
    
    output.positionW = mul(float4(vertex.position, 1.0), objectBuffer.world).xyz;
    output.position = mul(float4(output.positionW, 1.0), sceneBuffer.viewProjection);
    output.normal = mul(vertex.normal, (float3x3)objectBuffer.transposeInverseWorld);
    output.texCoord = vertex.texCoord;
    output.tangent = vertex.tangent.xyz;
    
    return output;
}

float4 PsMain(VOutput input) : SV_Target
{
    ConstantBuffer<SceneBuffer> sceneBuffer = ResourceDescriptorHeap[renderResource.sceneBufferIndex];
    Texture2D diffuse = ResourceDescriptorHeap[renderResource.diffuseTextureIndex];
    Texture2D metalRoughness = ResourceDescriptorHeap[renderResource.metallicRoughnessTextureIndex];
    
    float3 color = diffuse.Sample(gLinearWrap, input.texCoord).xyz;
    
    float3 Lo = float3(0.0, 0.0, 0.0);
    for (uint i = 0; i < lightBuffer.numLights; i++)
    {
        Light light = lightBuffer.lights[i];
        Lo += BlinnPhong(light, input.normal, input.positionW, sceneBuffer.viewPosition.xyz) * color;
    }
    
    return float4(Lo, 1.0);
}