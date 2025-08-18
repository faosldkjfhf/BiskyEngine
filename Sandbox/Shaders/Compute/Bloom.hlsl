struct RenderResource
{
    int outputImageIndex;
    int horizontal;
};
ConstantBuffer<RenderResource> renderResource : register(b0);

[numthreads(8, 8, 1)]
void CsMain(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    // -- weights for the gaussian blur
    const float weights[5] =
    {
        0.227027,
        0.1945946,
        0.1216216,
        0.054054,
        0.016216
    };
    
    RWTexture2D<float4> outputImage = ResourceDescriptorHeap[NonUniformResourceIndex(renderResource.outputImageIndex)];
    
    int2 texCoord = dispatchThreadId.xy;
    
    int2 imageSize;
    outputImage.GetDimensions(imageSize.x, imageSize.y);
    
    // -- texture offset
    float2 texOffset = 1.0 / imageSize;
    
    // -- current fragment's contribution
    float3 result = outputImage[texCoord].rgb * weights[0];
    
    // -- perform the gaussian blur
    if (renderResource.horizontal == 1)
    {
        for (int i = 1; i < 5; i++)
        {
            result += outputImage[texCoord + float2(texOffset.x * i, 0.0)].rgb * weights[i];
            result += outputImage[texCoord - float2(texOffset.x * i, 0.0)].rgb * weights[i];
        }
    }
    else
    {
        for (int i = 1; i < 5; i++)
        {
            result += outputImage[texCoord + float2(0.0, texOffset.y * i)].rgb * weights[i];
            result += outputImage[texCoord - float2(0.0, texOffset.y * i)].rgb * weights[i];
        }
    }
    
    // -- update value
    outputImage[texCoord] = float4(result, 1.0);

}