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
    int normalTextureIndex;
};

ConstantBuffer<SceneBuffer> sceneBuffer : register(b0);
ConstantBuffer<LightBuffer> lightBuffer : register(b1);
ConstantBuffer<ObjectBuffer> objectBuffer : register(b2);
ConstantBuffer<RenderResource> renderResource : register(b3);

VOutput VsMain(uint vertexId : SV_VertexID)
{
    StructuredBuffer<Vertex> vertexBuffer = ResourceDescriptorHeap[renderResource.vertexBufferIndex];
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
    Texture2D<float4> diffuse = ResourceDescriptorHeap[renderResource.diffuseTextureIndex];
    Texture2D<float4> metalRoughness = ResourceDescriptorHeap[renderResource.metallicRoughnessTextureIndex];
    Texture2D<float4> normal = ResourceDescriptorHeap[renderResource.normalTextureIndex];
    
    float3 color = float3(1.0, 1.0, 1.0);
    if (renderResource.diffuseTextureIndex >= 0)
    {
        color = diffuse.Sample(linearWrapSampler, input.texCoord).xyz;
    }

    float3 Lo = float3(0.0, 0.0, 0.0);
    for (uint i = 0; i < lightBuffer.numLights; i++)
    {
        Light light = lightBuffer.lights[i];
        Lo += BlinnPhong(light, input.normal, input.positionW, sceneBuffer.viewPosition.xyz) * color;
    }
    
    return float4(Lo, 1.0);
}