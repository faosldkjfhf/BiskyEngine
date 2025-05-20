struct VOutput
{
    float4 Position : SV_Position;
    float3 FragPosition : POSITION;
    float3 Normal : NORMAL;
};

struct ObjectConstants
{
    float4x4 World;
    float4x4 InverseWorld;
    float4x4 NormalMatrix;
};

struct PassConstants
{
    float4x4 View;
    float4x4 Projection;
    float4 ViewPosition;
};

ConstantBuffer<ObjectConstants> gObject : register(b0);
ConstantBuffer<PassConstants> gPass : register(b1);

// FIXME: Pass in lights through pass constants
float4 main(VOutput input) : SV_Target {
    float3 lightPos = float3(0.0, 3.0, 3.0);
    float3 lightStrength = float3(1.0, 1.0, 1.0);
    float3 material = float3(1.0, 0.5, 0.0);
    float4 finalColor = float4(0.0, 0.0, 0.0, 1.0);
    
    float ambientStrength = 0.3;
    float3 ambient = ambientStrength * lightStrength;
    
    float3 normal = normalize(input.Normal);
    float3 lightDir = normalize(lightPos - input.FragPosition);
    float diff = max(dot(normal, lightDir), 0.0);
    float3 diffuse = diff * lightStrength;
    
    // TODO: Specular not working
    float3 viewDir = normalize(gPass.ViewPosition.xyz - input.FragPosition);
    float3 halfwayDir = normalize(lightDir + viewDir);
    float3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(halfwayDir, normal), 0.0), 8);
    float3 specular = spec * lightStrength;
    
    float3 color = (ambient + diffuse + specular) * material;
    finalColor = float4(color, 1.0);
    return finalColor;
}