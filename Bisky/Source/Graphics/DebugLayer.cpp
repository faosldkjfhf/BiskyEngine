#include "Common.hpp"

#include "Graphics/DebugLayer.hpp"

namespace bisky::gfx
{

DebugLayer::DebugLayer()
{
#ifdef _DEBUG
    LOG_INFO("Debugging enabled");
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_d3d12Debug))))
    {
        m_d3d12Debug->EnableDebugLayer();
    }

    if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&m_dxgiDebug))))
    {
        m_dxgiDebug->EnableLeakTrackingForThread();
    }
#endif
}

DebugLayer::~DebugLayer()
{
#ifdef _DEBUG
    if (m_dxgiDebug)
    {
        m_dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL,
                                       DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
    }

    m_dxgiDebug.Reset();
    m_d3d12Debug.Reset();
#endif
}

} // namespace bisky::gfx