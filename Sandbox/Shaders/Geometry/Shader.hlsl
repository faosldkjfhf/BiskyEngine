#include "Lighting/PBR.hlsli"
#include "Lighting/BlinnPhong.hlsli"

struct VOutput
{
    float4 position : SV_Position;
    float3 positionW : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float3 tangent : TANGENT;
};

struct RenderResource
{
    int vertexBufferIndex;
    int diffuseTextureIndex;
    int metallicRoughnessTextureIndex;
    int normalTextureIndex;
};

ConstantBuffer<RenderResource> renderResource : register(b0);
ConstantBuffer<SceneBuffer> sceneBuffer : register(b1);
ConstantBuffer<LightBuffer> lightBuffer : register(b2);
ConstantBuffer<ObjectBuffer> objectBuffer : register(b3);

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
    Texture2D<float4> diffuseMap = ResourceDescriptorHeap[renderResource.diffuseTextureIndex];
    Texture2D<float4> metalRoughnessMap = ResourceDescriptorHeap[renderResource.metallicRoughnessTextureIndex];
    Texture2D<float4> normalMap = ResourceDescriptorHeap[renderResource.normalTextureIndex];
    
    // ----- Albedo -----
    const float gamma = 2.2;
    float3 albedo = float3(1.0, 1.0, 1.0);
    if (renderResource.diffuseTextureIndex >= 0)
    {
        // ----- Textures should be adjusted to linear space since they are usually sRGB -----
        albedo = diffuseMap.Sample(linearWrapSampler, input.texCoord).xyz;
        albedo = pow(albedo, gamma);
    }
    
    // ----- Roughness and metalness -----
    float roughness, metalness = 0.0;
    if (renderResource.metallicRoughnessTextureIndex >= 0)
    {
        roughness = metalRoughnessMap.Sample(linearWrapSampler, input.texCoord).g;
        metalness = metalRoughnessMap.Sample(linearWrapSampler, input.texCoord).b;
    }
    
    // ----- Normal vector and View direction -----
    float3 N = normalize(input.normal);
    float3 V = normalize(sceneBuffer.viewPosition.xyz - input.positionW);
    
    // ----- Calculate f0 -----
    float3 f0 = float3(0.04, 0.04, 0.04);
    f0 = lerp(f0, albedo, metalness);
    
    float3 Lo = float3(0.0, 0.0, 0.0);
    for (uint i = 0; i < lightBuffer.numLights; i++)
    {
        Light light = lightBuffer.lights[i];
        Lo += BlinnPhong(light, N, input.positionW, sceneBuffer.viewPosition.xyz) * albedo;
       
        /*
        // ----- Light direction and Halfway direction -----
        float3 L = normalize(light.position.xyz - input.positionW);
        float3 H = normalize(L + V);
        
        // ----- Calculate DGF -----
        float D = D_GGX(N, H, roughness);
        float G = G_Smith(N, V, L, roughness);
        float3 F = F_FresnelSchlick(max(dot(L, H), 0.0), f0);
        
        // ----- Calculate the diffuse contribution -----
        float3 kS = F;
        float3 kD = 1.0 - kS;
        kD *= 1.0 - metalness;
        
        // ----- Specular component -----
        float3 num = D * G * F;
        float3 denom = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 1e-5;
        float3 Fs = num / denom;
        
        // ----- Diffuse component (Lambertian) -----
        float3 Fd = albedo / PI;
        
        // ----- Accumulate outgoing light -----
        float NoL = max(dot(N, L), 0.0);
        Lo += (kD * Fd + Fs) * NoL * light.strength.xyz;
        */
    }
    
    /*
    // ----- Calculate ambient -----
    float3 ambient = float3(0.03, 0.03, 0.03) * albedo;
    Lo += ambient; 
    */
    
    return float4(Lo, 1.0);
}