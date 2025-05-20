#include "Common.h"

#include "DX12/Initializers.h"

namespace
{

static D3D12_HEAP_PROPERTIES hp;
static D3D12_RESOURCE_DESC rd;
static D3D12_RESOURCE_BARRIER rb;

} // namespace

namespace DX12
{

const D3D12_RESOURCE_BARRIER &ResourceBarrier::Transition(ID3D12Resource *pResource, D3D12_RESOURCE_STATES beginState,
                                                          D3D12_RESOURCE_STATES finalState)
{
  ZeroMemory(&rb, sizeof(D3D12_RESOURCE_BARRIER));
  rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  rb.Transition.pResource = pResource;
  rb.Transition.StateBefore = beginState;
  rb.Transition.StateAfter = finalState;
  rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  return rb;
}

const D3D12_RESOURCE_DESC &ResourceDesc::Buffer(UINT byteSize)
{
  ZeroMemory(&rd, sizeof(D3D12_RESOURCE_DESC));
  rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  rd.Width = byteSize;
  rd.Height = 1;
  rd.DepthOrArraySize = 1;
  rd.SampleDesc.Count = 1;
  rd.SampleDesc.Quality = 0;
  rd.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
  rd.MipLevels = 1;
  rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
  rd.Flags = D3D12_RESOURCE_FLAG_NONE;
  return rd;
}

const D3D12_HEAP_PROPERTIES &HeapProperties(D3D12_HEAP_TYPE type)
{
  ZeroMemory(&hp, sizeof(D3D12_HEAP_PROPERTIES));
  hp.Type = type;
  hp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
  hp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
  hp.CreationNodeMask = 0;
  hp.VisibleNodeMask = 0;
  return hp;
}

} // namespace DX12
