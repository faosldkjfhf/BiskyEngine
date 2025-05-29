#pragma once

#include "Common.h"

namespace D3D12
{

namespace ResourceBarrier
{

const D3D12_RESOURCE_BARRIER &Transition(ID3D12Resource *pResource, D3D12_RESOURCE_STATES beginState,
                                         D3D12_RESOURCE_STATES finalState);

}

namespace ResourceDesc
{

const D3D12_RESOURCE_DESC &Buffer(UINT byteSize);
const D3D12_RESOURCE_DESC &Texture2D(UINT width, UINT height, DXGI_FORMAT format);

} // namespace ResourceDesc

const D3D12_HEAP_PROPERTIES &HeapProperties(D3D12_HEAP_TYPE type);

} // namespace D3D12
