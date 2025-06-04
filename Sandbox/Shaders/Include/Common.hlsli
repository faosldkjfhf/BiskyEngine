#pragma once

struct ObjectBuffer
{
    float4x4 world;
    float4x4 inverseWorld;
    float4x4 transposeInverseWorld;
};

struct SceneBuffer
{
    float4x4 view;
    float4x4 projection;
    float4x4 viewProjection;
    float4 viewPosition;
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

SamplerState linearWrapSampler: register(s0);