#include "Common.h"

#include "DX12/DebugLayer.h"

namespace DX12
{

DebugLayer::Error DebugLayer::Init()
{
#ifdef _DEBUG
  if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&md3dDebug))))
  {
    md3dDebug->EnableDebugLayer();
    if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&mdxgiDebug))))
    {
      mdxgiDebug->EnableLeakTrackingForThread();
      return None;
    }
  }

  return Failed;
#endif

  return None;
}

void DebugLayer::Shutdown()
{
#ifdef _DEBUG
  if (mdxgiDebug)
  {
    mdxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL,
                                  DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
  }

  mdxgiDebug.Reset();
  md3dDebug.Reset();
#endif
}

} // namespace DX12
