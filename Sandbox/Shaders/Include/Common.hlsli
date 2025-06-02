struct ObjectBuffer
{
    float4x4 world;
    float4x4 inverseWorld;
};

struct SceneBuffer
{
    float4x4 view;
    float4x4 projection;
    float4x4 viewProjection;
};

struct MaterialInfo
{
    int diffuseTextureIndex;
    int metallicRoughnessTextureIndex;
};

struct RenderResource
{
    int vertexBufferIndex;
    int sceneBufferIndex;
    int diffuseTextureIndex;
    int metallicRoughnessTextureIndex;
};

struct Light
{
    float4 position;
    float4 strength;
};

struct LightBuffer
{
    Light lights[10];
    int numLights;
};