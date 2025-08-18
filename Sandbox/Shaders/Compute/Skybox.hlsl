struct RenderResource
{
    int outputImageIndex;
};
ConstantBuffer<RenderResource> renderResource : register(b0);

[numthreads(8, 8, 1)]
void CsMain(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    // -- retrieve output image
    RWTexture2D<float4> outputImage = ResourceDescriptorHeap[renderResource.outputImageIndex];
    
    // -- get texel coord
    uint2 texelCoord = dispatchThreadId.xy;
    
    // -- get image size
    // uint2 imageSize;
    // outputImage.GetDimensions(imageSize.x, imageSize.y);
    
    // -- set pixel color
    outputImage[dispatchThreadId.xy] = float4(1.0, 0.5, 0.5, 1.0);
}