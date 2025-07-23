struct RenderResource
{
    int sourceImageIndex;
    int outputImageIndex;
};

ConstantBuffer<RenderResource> renderResource : register(b0);

[numthreads(8, 8, 1)]
void CsMain(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    Texture2D<float4> sourceImage = ResourceDescriptorHeap[renderResource.sourceImageIndex];
    RWTexture2D<float4> outputImage = ResourceDescriptorHeap[renderResource.outputImageIndex];
    
    outputImage[dispatchThreadId.xy] = float4(1.0, 0.5, 0.5, 1.0);
}