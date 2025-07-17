#pragma once

#include "Graphics/Resources.hpp"
#include "Graphics/ShaderCompiler.hpp"
#include <wrl.h>

namespace bisky::gfx
{

/*
 * A wrapper class around a d3d12 pipeline state.
 */
class PipelineState
{
  public:
    explicit PipelineState(ID3D12Device10 *const device, const GraphicsPipelineStateDesc &gfxD);
    ~PipelineState();

    PipelineState(const PipelineState &)                    = delete;
    const PipelineState &operator=(const PipelineState &)   = delete;
    PipelineState(const PipelineState &&)                   = delete;
    const PipelineState &&operator=(const PipelineState &&) = delete;

  public:
    ID3D12PipelineState *const getPipelineState() const;

  private:
    explicit PipelineState() = default;

    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
};

} // namespace bisky::gfx