#pragma once

#include "Common.h"

namespace DX12
{

class DebugLayer
{
public:
  enum Error
  {
    None = 0,
    Failed = 1,
  };

  Error Init();
  void Shutdown();

private:
#ifdef _DEBUG
  ComPtr<ID3D12Debug6> md3dDebug;
  ComPtr<IDXGIDebug1> mdxgiDebug;
#endif

public:
  DebugLayer(const DebugLayer &) = delete;
  const DebugLayer &operator=(const DebugLayer &) = delete;
  inline static DebugLayer &Get()
  {
    static DebugLayer instance;
    return instance;
  }

private:
  DebugLayer() = default;
};

} // namespace DX12
