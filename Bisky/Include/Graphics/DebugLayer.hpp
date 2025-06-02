#pragma once

#include "Common.hpp"

namespace bisky::gfx
{

/*
 * This is a basic debug wrapper.
 * Meant only to be used when _DEBUG is defined.
 */
class DebugLayer
{
  public:
    /*
     * Initializes the D3D12 Debug and the DXGI Debug.
     * Only does something when _DEBUG is defined.
     */
    explicit DebugLayer();

    /*
     * Resets the D3D12 Debug and DXGI Debug.
     * Outputs currently live objects to the Debug window.
     * Only does something when _DEBUG is defined.
     */
    ~DebugLayer();

    DebugLayer(const DebugLayer &)                    = delete;
    const DebugLayer &operator=(const DebugLayer &)   = delete;
    DebugLayer(const DebugLayer &&)                   = delete;
    const DebugLayer &&operator=(const DebugLayer &&) = delete;

  private:
#ifdef _DEBUG
    wrl::ComPtr<ID3D12Debug6> m_d3d12Debug;
    wrl::ComPtr<IDXGIDebug1>  m_dxgiDebug;
#endif
};

} // namespace bisky::gfx